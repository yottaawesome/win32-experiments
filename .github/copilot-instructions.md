# Win32 Experiments

## Building and testing

Use Windows with Visual Studio 2026 or its Build Tools, the **Desktop development with C++** workload, and a Windows SDK. The current solution uses MSVC `v145`; follow the checked-in `.vcxproj` settings where older READMEs differ. Use `Debug|x64` by default. The solution also exposes `Release|x64`, but the README only vouches for x64 and does not assume parity between configurations. Modern module-based samples such as `CPU_ID` and `MultiControlSamples` use `stdcpplatest` and `BuildStlModules` for `import std`.

From the repository root in Developer PowerShell for Visual Studio, build and smoke-test one experiment:

```powershell
msbuild .\src\Hello\Hello.vcxproj '/p:Configuration=Debug;Platform=x64'
.\src\Hello\x64\Debug\Hello.exe
```

`Hello` should print `seconds since epoch:` followed by a number. For changes, build the affected `.vcxproj` and exercise that sample's behavior. To build the full solution, run `msbuild .\src\win32-experiments.sln '/p:Configuration=Debug;Platform=x64'`.

Use the executable path printed by MSBuild: solution builds normally put binaries in `src\x64\Debug`, while standalone builds normally use the project's own `x64\Debug` directory. Projects with solution-level prerequisites should be built through the solution or with those prerequisites built first.

- `Detours` uses the MSBuild vcpkg manifest at `src\Detours\vcpkg.json`.
- For WebSocket integration checks, run `npm ci` in the relevant server directory. Start `src\WebSocket\server` with `npm start`, or `src\WebSocketProto\TestNodeServer` with `npm run start2`. Both servers expect local `key.pem` and `cert.pem` files in their working directory and listen on `127.0.0.1:51935`.

## Architecture

- This is a lab of separate native Win32 API experiments, not one application. `src\win32-experiments.sln` groups the samples by API area; other variants, such as `WebSocketProto` and `WinsockCoroutineJunie`, are standalone `.vcxproj` builds. Scope changes to the experiment and its actual dependencies.
- Check `.vcxproj` `ProjectReference` entries and solution dependencies before treating a sample as self-contained. For example, `WinSockClient` and `WinSockServer` share `BasicClientServerShared`, while `SelfExtractingExe` embeds `DLLToExtract.dll` as a resource using a pre-build copy from the solution's `x64\Debug` output.
- UI experiments are independent implementations. `MultiControlSamples` is built from `src\ProgressBar\ProgressBar.vcxproj`, separate from `src\progress-bar`. Its `App` composes a top-level window, a child panel holding the button and progress bar, and a status bar. `CPU_ID` instead splits CPUID feature detection (`InstructionSet`) from the `Application` panels and reusable `UI` wrappers, behind the `CpuId` module.
- Check sample READMEs for runtime setup: `basic-com-client` needs the built `basic-com` DLL registered and uses apartment-threaded COM; DesktopToasts needs a Start menu shortcut with an AppUserModelID. For `WebSocketProto` schema changes, regenerate both the C++ and Node bindings from `testmessage.proto` using the sample README.

## Code conventions

- Under `src`, `.editorconfig` specifies UTF-8 and tab indentation. `.vcxproj` files enumerate sources explicitly: add new `.cpp` and `.ixx` files as `ClCompile` items, and re-export public module partitions through the owning interface (e.g. `CpuId.ixx` or `MultiControlSamples.ixx`).
- In newer module-based samples, SDK headers belong in the global module fragment of the sample's platform module. Use its exported `Win32::` types, constants, and Unicode `...W` APIs. `CPU_ID` and `ProgressBar` have separate `Win32` modules; the `src\Win32` library exports `common` instead. Follow the sample's existing imports rather than assuming a shared wrapper.
- In the newer UI wrappers, native callbacks borrow stable C++ object addresses. Custom windows bind the pointer in `GWLP_USERDATA` on `WM_NCCREATE` and clear it on `WM_NCDESTROY`; built-in controls use `SetWindowSubclass` reference data. In `CPU_ID`, `WindowClass::CreateWindow` creates the `unique_ptr` before calling `CreateWindowExW` and returns ownership to the caller. Move the owner, not the registered window, and do not delete the object from the window procedure.
- Keep message routing local to the wrapper: a parent reflects `WM_COMMAND` (and `WM_NOTIFY` in `MultiControlSamples`) to the originating control, and unhandled messages fall through to `DefWindowProcW` or `DefSubclassProc` as appropriate. In `CPU_ID`, exceptions at callbacks or the application entry point go through `Error::ErrorExit`, which writes diagnostics to stderr and attempts a temp log before calling `std::_Exit` without teardown.
- Samples are experiments rather than production templates. When changing a Win32 call, check its Windows SDK documentation for API-specific requirements and supported Windows versions.
