# VectoredExceptionHandler

A small sample that demonstrates using `AddVectoredExceptionHandler()` to
intercept hardware and software exceptions process-wide, and using the
`PEXCEPTION_POINTERS` argument to pull meaningful detail out of the fault —
**including** identifying an MSVC C++ `throw` and decoding the thrown
object's type.

Build with Visual Studio 2022 (v143 toolset, C++ latest). Link is against
`dbghelp.lib` for symbol resolution. x64 is the primary target.

---

## What is a Vectored Exception Handler?

Structured Exception Handling (SEH) is frame-based — handlers are pushed
as the thread's call stack grows, and the OS walks them during an
exception. A **Vectored Exception Handler** is different:

- It is registered for the **whole process**. Every thread, now and in
  the future, is covered by the same handler.
- It runs **before** the per-thread SEH frame chain is consulted (when
  registered with `FirstHandler = 1`).
- Multiple VEHs can be installed and are called in order until one
  returns `EXCEPTION_CONTINUE_EXECUTION`, or until the list is exhausted
  and SEH / the debugger gets the first-chance exception.

The companion API `AddVectoredContinueHandler` installs *continue*
handlers which run after SEH if the exception wasn't handled — useful
for crash reporters that want to log and then let the process die.

Refs:
- [AddVectoredExceptionHandler — MSDN](https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-addvectoredexceptionhandler)
- [Vectored Exception Handling — MSDN](https://learn.microsoft.com/en-us/windows/win32/debug/vectored-exception-handling)

---

## Anatomy of `PEXCEPTION_POINTERS`

```c
typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;   // what happened
    PCONTEXT          ContextRecord;     // CPU state when it happened
} EXCEPTION_POINTERS;
```

### `EXCEPTION_RECORD`

| Field | Meaning |
|---|---|
| `ExceptionCode` | The SEH code, e.g. `EXCEPTION_ACCESS_VIOLATION` (`0xC0000005`). |
| `ExceptionFlags` | Bit flags. `EXCEPTION_NONCONTINUABLE` = you can't resume. |
| `ExceptionAddress` | The faulting instruction's address. Symbolize this. |
| `NumberParameters` | How many entries in `ExceptionInformation` are valid. |
| `ExceptionInformation[]` | Code-specific extra data (see below). |
| `ExceptionRecord` | Pointer to a nested record (e.g. a rethrown exception). |

### Code-specific payloads

- **`EXCEPTION_ACCESS_VIOLATION`** (and `EXCEPTION_IN_PAGE_ERROR`):
  - `[0]` = 0 (read), 1 (write), or 8 (DEP / no-execute).
  - `[1]` = the bad virtual address.
- **`EXCEPTION_INT_DIVIDE_BY_ZERO`, `EXCEPTION_STACK_OVERFLOW`, `EXCEPTION_ILLEGAL_INSTRUCTION`, `EXCEPTION_BREAKPOINT` …**: no extra parameters, just the code + address.

### `CONTEXT`

The full CPU register file at the fault point. Use it to:
- Print `Rip`/`Rsp`/`Rbp` (x64) or `Eip`/`Esp`/`Ebp` (x86).
- Feed `StackWalk64` / `RtlVirtualUnwind` for a stack trace starting at
  the fault. For a demo we use `RtlCaptureStackBackTrace`, which is
  simpler and doesn't need the `CONTEXT`.
- Pair with `MINIDUMP_EXCEPTION_INFORMATION` when writing a minidump via
  `MiniDumpWriteDump`.

---

## Detecting an MSVC C++ exception

MSVC implements `throw` by raising an SEH exception with code
**`0xE06D7363`** — low three bytes are ASCII `"msc"`. When you see that
code, `ExceptionInformation` encodes the thrown object:

| Index | Meaning |
|---|---|
| `[0]` | Magic. `0x19930520`, `0x19930521`, or `0x19930522`. |
| `[1]` | Pointer to the thrown object (lives in the throwing frame). |
| `[2]` | Pointer to `ThrowInfo` (type info + catchable types + dtor). |
| `[3]` | **x64 only** — image base that the RVAs inside `ThrowInfo` are relative to. |

On **x64**, every "pointer" inside `ThrowInfo`, `CatchableTypeArray`,
and `CatchableType` is a 32-bit **RVA** — add it to
`ExceptionInformation[3]` to get a real pointer. On **x86** those fields
are raw pointers and `[3]` does not exist.

### Walking the type info

```
ThrowInfo
  └─ pCatchableTypeArray  ──▶ CatchableTypeArray
                                └─ arrayOfCatchableTypes[0..N-1]
                                     └─ CatchableType
                                          ├─ sizeOrOffset   (sizeof the object)
                                          ├─ copyFunction   (copy ctor RVA, or 0)
                                          └─ pType          ──▶ TypeDescriptor
                                                                  └─ name  (mangled)
```

`arrayOfCatchableTypes[0]` is the **most-derived** type. The rest are
base classes in the order the MSVC runtime uses them to match
`catch (Base&)` clauses.

`TypeDescriptor::name` is a mangled MSVC name such as
`.?AVruntime_error@std@@`. Pass it to `UnDecorateSymbolName`
(`dbghelp.lib`) or `__unDName` for a readable form.

### Recovering `what()` safely

If **any** entry in the catchable-type list is `std::exception`, then
casting the thrown object to `const std::exception*` and calling
`what()` is well-defined — the runtime already encoded the full base
chain needed for `catch (std::exception&)`. This is what
`DecodeCppException` in the sample does.

Declarations for `ThrowInfo`, `CatchableTypeArray`, `CatchableType`, and
`TypeDescriptor` live in the MSVC-shipped header `<ehdata.h>`.

---

## What this program does

```text
VectoredExceptionHandler sample, PID=... TID=...

--- Demo: C++ throw ---
=== VEH fired on thread ... ===
  code    : 0xE06D7363  (MSVC C++ exception (0xE06D7363))
  ...
  [C++] 2 catchable type(s):
    [0] class MyError   (sizeof=...)   <-- most-derived type
    [1] class std::runtime_error   ...
    ...
  [C++] what(): something went wrong
main caught: something went wrong

--- Demo: access violation ---
=== VEH fired on thread ... ===
  code    : 0xC0000005  (EXCEPTION_ACCESS_VIOLATION)
  AV: read at 0x0000000000000000
  ...
main __except swallowed the AV

--- Demo: integer divide by zero ---
...

--- Demo: C++ throw on worker thread ---
=== VEH fired on thread <worker tid> ===
...
```

The last demo proves that a single VEH installed on the main thread
also fires for exceptions on a worker thread — VEH is process-wide.

---

## Do / Don't

**Do:**
- Return `EXCEPTION_CONTINUE_SEARCH` for exceptions you're merely
  observing. Let SEH and C++ `catch` blocks do their job.
- Use VEH for crash telemetry and diagnostics: minidump on fatal codes,
  log first-chance exceptions during development.
- `RemoveVectoredExceptionHandler` if the handler's lifetime is shorter
  than the process (e.g. installed from a DLL).

**Don't:**
- Return `EXCEPTION_CONTINUE_EXECUTION` unless you actually repaired the
  cause (e.g. committed a reserved page).
- Allocate, take locks the faulting thread might hold, or `throw` inside
  the handler. VEH runs in the faulting thread's context with the CPU
  state frozen; keep it boring.
- Forget that VEH sees *every* C++ `throw` as a first-chance exception,
  even ones the program handles normally. This can be noisy.

---

## Is this used in real production apps?

Yes — but usually a narrower subset than what this sample shows. In
practice, **VEH + minidump on crash** is the dominant pattern on
Windows. The other techniques here (C++ type decode, `what()` recovery,
in-process symbolization) are more common in dev builds, QA harnesses,
and specialized tooling than in shipping binaries.

### Very common in production

- **Crash reporters.** `SetUnhandledExceptionFilter` (often) plus
  `AddVectoredExceptionHandler` (sometimes, as first handler) to call
  `MiniDumpWriteDump` on fatal codes and terminate. Google's
  **Crashpad** and **Breakpad** (Chrome, Firefox, many games, Electron)
  are the canonical examples. They usually write the dump
  **out-of-process** so a corrupted heap/stack can't break the
  reporter.
- **Watching for specific fatal codes:** access violation, stack
  overflow, illegal instruction, heap corruption
  (`STATUS_HEAP_CORRUPTION`), fail-fast
  (`STATUS_STACK_BUFFER_OVERRUN`). Write the dump, upload on next
  launch.
- **Guard-page / JIT tricks.** VEH that returns
  `EXCEPTION_CONTINUE_EXECUTION` after committing a reserved page.
  Used by GC runtimes (V8, CoreCLR, Mono), JITs, and software that
  implements stack-like growable buffers. This is one of the few
  legitimate uses of `CONTINUE_EXECUTION`.

### Common in tooling, rarer in shipping apps

- **First-chance exception logging** during development or QA — useful
  for catching swallowed exceptions, but too noisy/expensive for
  release.
- **In-process symbolization via DbgHelp.** Debuggers, profilers
  (PerfView, ETW consumers), and internal diagnostic tools do this.
  Shipping apps usually strip PDBs, upload a minidump, and symbolize
  **server-side** against an archived symbol server. DbgHelp is also
  single-threaded and has a big surface area, which is another reason
  to keep it out of the crashing process.
- **Stack walking in the handler** (`StackWalk64`, `RtlVirtualUnwind`).
  Crash libraries do this internally; most teams just let
  `MiniDumpWriteDump` capture state and analyze later.

### Rare in production

- **Decoding `ThrowInfo` / `CatchableType` to recover the thrown C++
  type or `what()`.** It works — this sample proves it — but:
  - It's undocumented and has shifted over time (hence the three magic
    numbers `0x19930520/21/22`).
  - `std::current_exception()` + `std::rethrow_exception` inside a
    `std::set_terminate` handler gives you equivalent info portably,
    and is what most apps reach for when they want the type of an
    *unhandled* C++ exception.
  - Observing *every* `throw` as a first-chance SEH exception is noisy
    and has measurable cost (kernel round-trip, context capture on
    every throw).
- So the `ThrowInfo` walk mainly appears in: debuggers (WinDbg, Visual
  Studio), anti-cheat / DRM, EH-runtime replacements, diagnostic
  libraries (e.g. StackWalker, cpptrace), and research / instrumentation
  tools — not in your average line-of-business app.

### Typical production recipe

1. Install `SetUnhandledExceptionFilter` and/or a last-resort VEH.
2. On a fatal code, hand off to an **out-of-process** helper (e.g. the
   Crashpad handler) that writes the minidump. In-process dumping is
   unreliable when the process is already corrupt.
3. Also hook the paths that bypass SEH: `std::set_terminate`,
   `_set_purecall_handler`, `_set_invalid_parameter_handler`,
   `signal(SIGABRT, ...)`, and fail-fast.
4. Strip PDBs from shipped binaries; upload them to a symbol server.
5. Symbolize and analyze dumps offline — WinDbg, Visual Studio, or an
   automated pipeline (Sentry, Backtrace, BugSplat, etc.).

### Where this sample's *full* approach really is used

- Anti-cheat / DRM / obfuscation — VEH to intercept debugger
  breakpoints, hide from debuggers, or implement code virtualization.
- Mixed-mode runtimes (.NET, Java on Windows, Node/V8) — VEH for JIT
  page faults, stack probes, null-check elision.
- Internal dev / test builds of large apps — full first-chance logging
  with C++ type decode to catch swallowed exceptions.
- Debugging and profiling libraries themselves.

**Bottom line:** the technique is real and shipped, but most apps use
only the crash-reporter slice of it, and leave type decoding and
symbolization to offline tooling.

---

## Files

- `VectoredExceptionHandler.cpp` — the demo + the handler.
- `VectoredExceptionHandler.vcxproj` — x64/x86 Debug/Release, v143
  toolset, `stdcpplatest`, links `dbghelp.lib`.
- `README.md` — this file.
