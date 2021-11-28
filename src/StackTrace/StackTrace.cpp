#include <iostream>
#include <Windows.h>
#include <vector>
#include <memory>
#include <algorithm>
#include <format>
#include <dbghelp.h>
#include "StackTracer.h"

#pragma comment(lib, "dbghelp.lib")

// See https://stackoverflow.com/questions/6205981/windows-c-stack-trace-from-a-running-app
// https://docs.microsoft.com/en-us/windows/win32/debug/using-dbghelp
// https://stackoverflow.com/questions/590160/how-to-log-stack-frames-with-windows-x64
// https://stackoverflow.com/questions/136752/how-do-you-make-stackwalk64-work-successfully-on-x64?rq=1
// https://stackoverflow.com/questions/5705650/stackwalk64-on-windows-get-symbol-name
// https://docs.microsoft.com/en-us/windows/win32/debug/retrieving-symbol-information-by-address
void PrintStackWalk64(const unsigned skipFrameCount)
{
    // https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-syminitialize
    HANDLE process = GetCurrentProcess();
    if (!SymInitialize(process, nullptr, true))
    {
        std::wcerr << "SymInitialize() failed\n";
        return;
    }

    // https://docs.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-rtlcapturecontext
    CONTEXT context;
    RtlCaptureContext(&context);

    constexpr static int MaxFunctionNameLength = 256;
    std::vector<std::byte> symbolInfoBytes(sizeof(SYMBOL_INFOW) + MaxFunctionNameLength * sizeof(wchar_t));
    SYMBOL_INFOW* symbol = reinterpret_cast<SYMBOL_INFOW*>(&symbolInfoBytes[0]);
    symbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
    symbol->MaxNameLen = MaxFunctionNameLength;

    // https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/ns-dbghelp-stackframe64
    STACKFRAME64 stack{ 0 };
    stack.AddrPC.Offset = context.Rip;
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrStack.Offset = context.Rsp;
    stack.AddrStack.Mode = AddrModeFlat;
    stack.AddrFrame.Offset = context.Rdi;
    stack.AddrFrame.Mode = AddrModeFlat;

    HANDLE thread = GetCurrentThread();
    for (unsigned frame = 0;; frame++)
    {
        const bool result = StackWalk64
        (
            IMAGE_FILE_MACHINE_AMD64, // https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_file_header
            process,
            thread,
            &stack,
            &context,
            nullptr,
            SymFunctionTableAccess64,
            SymGetModuleBase64,
            nullptr
        );
        if (!result)
        {
            std::wcerr << "StackWalk64() failed\n";
            break;
        }
        // Skip logging the first frame; it's just this function and we don't care about it
        if (frame == 0 || skipFrameCount >= frame)
            continue;

        // SymGetSymFromAddr64() is deprecated
        // https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-symfromaddr
        if (DWORD64 displacement = 0; !SymFromAddrW(process, stack.AddrPC.Offset, &displacement, symbol))
        {
            std::wcerr << "SymGetSymFromAddr64() failed\n";
            break;
        }

        // Not sure if we need this, except for DLLs
        // See SYMOPT_UNDNAME in https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-symsetoptions
        // https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-undecoratesymbolname
        std::wstring undecoratedName(MaxFunctionNameLength, '\0');
        const bool undecoratedSuccessfully = UnDecorateSymbolNameW(
            symbol->Name, 
            &undecoratedName[0], 
            static_cast<DWORD>(undecoratedName.size()), 
            UNDNAME_COMPLETE
        );
        if (!undecoratedSuccessfully)
            undecoratedName = L"<unknown>";

        std::wstring fileName = L"<unknown>";
        DWORD lineNumber = 0;
        IMAGEHLP_LINEW64 line{ .SizeOfStruct = sizeof(IMAGEHLP_LINEW64) };
        if (DWORD displacement = 0; SymGetLineFromAddrW64(process, stack.AddrPC.Offset, &displacement, &line))
        {
            fileName = line.FileName;
            lineNumber = line.LineNumber;
        }

        // Skip logging external frames
        if (std::wstring_view(symbol->Name) == L"invoke_main")
            break;
        std::wcout << std::format(
            L"{}() -> {}(): {:#X} in {}:{}\n",
            symbol->Name,
            undecoratedSuccessfully ? undecoratedName : L"<unknown>",
            symbol->Address,
            fileName,
            lineNumber
        );
    }

    if (!SymCleanup(process))
        std::wcerr << "SymCleanup() failed\n";
}

void PrintStackCpp(const unsigned skipFrameCount)
{
    void* stack[100];
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-rtlcapturestackbacktrace
    unsigned short frames = RtlCaptureStackBackTrace(0, 100, stack, nullptr);
    if (frames == 0)
    {
        std::wcerr << L"PrintStackCpp() did not capture any frames\n";
        return;
    }

    HANDLE process = GetCurrentProcess();
    if (!SymInitialize(process, nullptr, true))
    {
        std::wcerr << L"SymInitialize() failed\n";
        return;
    }

    constexpr static unsigned MaxFunctionNameLength = 256; // 255 + 1 terminating null
    // Don't use a smart pointer typed as SYMBOL_INFO, as memory would be leaked due to SYMBOL_INFOW::Name
    std::vector<std::byte> symbolInfoBytes(sizeof(SYMBOL_INFOW) + MaxFunctionNameLength * sizeof(wchar_t));
    SYMBOL_INFOW* symbol = reinterpret_cast<SYMBOL_INFOW*>(&symbolInfoBytes[0]);
    symbol->MaxNameLen = MaxFunctionNameLength;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
    IMAGEHLP_LINEW64 line{ .SizeOfStruct = sizeof(IMAGEHLP_LINEW64) };

    // Skip logging the first frame; it's just this function and we don't care about it
    for (unsigned i = 1; i < frames; i++)
    {
        if (skipFrameCount >= i)
            continue;

        const DWORD64 address = reinterpret_cast<DWORD64>(stack[i]);
        if (!SymFromAddrW(process, address, 0, symbol))
        {
            std::wcerr << "SymFromAddr() failed\n";
            continue;
        }
        if (DWORD displacement; !SymGetLineFromAddrW64(process, address, &displacement, &line))
        {
            std::wcerr << "SymGetLineFromAddr64() failed\n";
            continue;
        }
        // Skip logging external frames
        if (std::wstring_view(symbol->Name) == L"invoke_main")
            break;

        std::wstring undecoratedName(MaxFunctionNameLength, '\0');
        const bool undecoratedSuccessfully = UnDecorateSymbolNameW(
            symbol->Name, 
            &undecoratedName[0], 
            static_cast<DWORD>(undecoratedName.size()), 
            UNDNAME_COMPLETE
        );
        std::wcout << std::format(
            L"{}() -> {}(): {:#X} in {}:{}\n",
            symbol->Name,
            undecoratedSuccessfully ? undecoratedName : L"<unknown>",
            symbol->Address,
            line.FileName,
            line.LineNumber
        );
    }

    if (!SymCleanup(process))
        std::wcerr << L"SymCleanup() failed\n";
}

void Blah()
{
    PrintStackWalk64(0);
    //PrintStackCpp(0);
    //PrintStack();
    //PrintStackTrace();
}

int main(int argc, char* args[])
{
    Blah();
    return 0;
}
