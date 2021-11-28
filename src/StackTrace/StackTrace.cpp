#include <iostream>
#include <Windows.h>
#include <vector>
#include <algorithm>
#include <format>
#include <dbghelp.h>
#include "StackTracer.h"

#pragma comment(lib, "dbghelp.lib")

// See https://stackoverflow.com/questions/6205981/windows-c-stack-trace-from-a-running-app
// https://stackoverflow.com/questions/590160/how-to-log-stack-frames-with-windows-x64
// https://stackoverflow.com/questions/136752/how-do-you-make-stackwalk64-work-successfully-on-x64?rq=1
// https://stackoverflow.com/questions/5705650/stackwalk64-on-windows-get-symbol-name
void PrintStackWalk64(const unsigned skipFrameCount)
{
    // https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-syminitialize
    HANDLE process = GetCurrentProcess();
    if (!SymInitialize(process, nullptr, true))
    {
        std::cerr << "SymInitialize() failed\n";
        return;
    }
    // https://docs.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-rtlcapturecontext
    CONTEXT context;
    RtlCaptureContext(&context);
    HANDLE thread = GetCurrentThread();

    constexpr int MaxFunctionNameLength = 256;
    std::vector<std::byte> symbolInfoBytes(sizeof(SYMBOL_INFO) + MaxFunctionNameLength * sizeof(char));
    SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(&symbolInfoBytes[0]);
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = MaxFunctionNameLength;

    // https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/ns-dbghelp-stackframe64
    STACKFRAME64        stack{ 0 };
    stack.AddrPC.Offset = context.Rip;
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrStack.Offset = context.Rsp;
    stack.AddrStack.Mode = AddrModeFlat;
    stack.AddrFrame.Offset = context.Rdi;
    stack.AddrFrame.Mode = AddrModeFlat;

    for (unsigned frame = 0; ; frame++)
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
            std::cerr << "StackWalk64() failed\n";
            break;
        }
        // Skip logging the first frame; it's just this function and we don't care about it
        if (frame == 0 || skipFrameCount >= frame)
            continue;

        // SymGetSymFromAddr64() is deprecated
        // https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-symfromaddr
        if (DWORD64 displacement = 0; !SymFromAddr(process, stack.AddrPC.Offset, &displacement, symbol))
        {
            std::cerr << "SymGetSymFromAddr64() failed\n";
            break;
        }

        // Not sure if we need this, except for DLLs
        // See SYMOPT_UNDNAME in https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-symsetoptions
        // https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-undecoratesymbolname
        std::string undecoratedName(MaxFunctionNameLength, '\0');
        if (!UnDecorateSymbolName(symbol->Name, &undecoratedName[0], static_cast<DWORD>(undecoratedName.size()), UNDNAME_COMPLETE))
            undecoratedName = "<unknown>";

        std::string fileName = "<unknown>";
        DWORD lineNumber = 0;
        IMAGEHLP_LINE64 line{ .SizeOfStruct = sizeof(IMAGEHLP_LINE64) };
        if (DWORD displacement = 0; SymGetLineFromAddr64(process, stack.AddrPC.Offset, &displacement, &line))
        {
            fileName = line.FileName;
            lineNumber = line.LineNumber;
        }

        // Skip logging external frames
        if (std::string_view(symbol->Name) == "invoke_main")
            break;
        std::cout << std::format(
            "{}() -> {}(): {:#X} in {}:{}\n",
            symbol->Name,
            undecoratedName,
            symbol->Address,
            fileName,
            lineNumber);
    }

    if (!SymCleanup(process))
        std::cerr << "SymCleanup() failed\n";
}

void PrintStackCpp(const unsigned skipFrameCount)
{
    void* stack[100];
    unsigned short frames = RtlCaptureStackBackTrace(0, 100, stack, nullptr);
    if (frames == 0)
    {
        std::cerr << "PrintStackCpp() did not capture any frames\n";
        return;
    }

    HANDLE process = GetCurrentProcess();
    if (!SymInitialize(process, nullptr, true))
    {
        std::cerr << "SymInitialize() failed\n";
        return;
    }

    constexpr unsigned MaxFunctionNameLength = 256; // 255 + 1 terminating null
    std::vector<std::byte> symbolInfoBytes(sizeof(SYMBOL_INFO) + MaxFunctionNameLength * sizeof(char));
    SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(&symbolInfoBytes[0]);
    symbol->MaxNameLen = MaxFunctionNameLength;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    IMAGEHLP_LINE64 line{ .SizeOfStruct = sizeof(IMAGEHLP_LINE64) };

    // Skip logging the first frame; it's just this function and we don't care about it
    for (unsigned i = 1; i < frames; i++)
    {
        if (skipFrameCount >= i)
            continue;

        const DWORD64 address = reinterpret_cast<DWORD64>(stack[i]);
        if (!SymFromAddr(process, address, 0, symbol))
        {
            std::cerr << "SymFromAddr() failed\n";
            continue;
        }
        if (DWORD displacement; !SymGetLineFromAddr64(process, address, &displacement, &line))
        {
            std::cerr << "SymGetLineFromAddr64() failed\n";
            continue;
        }
        // Skip logging external frames
        if (std::string_view(symbol->Name) == "invoke_main")
            break;

        std::string undecoratedName(MaxFunctionNameLength, '\0');
        const bool undecoratedSuccessfully = UnDecorateSymbolName(
            symbol->Name, 
            &undecoratedName[0], 
            static_cast<DWORD>(undecoratedName.size()), 
            UNDNAME_COMPLETE
        );
        std::cout << std::format(
            "{}() -> {}(): {:#X} in {}:{}\n",
            symbol->Name,
            undecoratedSuccessfully ? undecoratedName : "<unknown>",
            symbol->Address,
            line.FileName,
            line.LineNumber
        );
    }

    if (!SymCleanup(process))
        std::cerr << "SymCleanup() failed\n";
}


// From https://stackoverflow.com/questions/5693192/win32-backtrace-from-c-code
void PrintStack()
{
    constexpr int MaxFunctionNameLength = 255;
    void* stack[100];
    unsigned short frames = CaptureStackBackTrace(0, 100, stack, nullptr);

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, true);

    // These two lines are equivalent
    SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + (MaxFunctionNameLength + 1) * sizeof(char), 1);
    //SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(1, sizeof(SYMBOL_INFO) + (MaxFunctionNameLength + 1) * sizeof(char));
    if (symbol == nullptr)
        return;
    
    symbol->MaxNameLen = MaxFunctionNameLength;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    IMAGEHLP_LINE64* line = (IMAGEHLP_LINE64*)malloc(sizeof(IMAGEHLP_LINE64));
    if (line == nullptr)
        return;

    memset(line, 0, sizeof(IMAGEHLP_LINE64));
    line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    DWORD displacement;
    for (unsigned int i = 0; i < frames; i++)
    {
        DWORD64 address = (DWORD64)(stack[i]);
        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
        if (SymGetLineFromAddr64(process, address, &displacement, line))
            std::cout << "Symbol: " << symbol->Name << " in " << line->FileName << ": line: " << std::dec << line->LineNumber << ": address: " << std::hex << symbol->Address << std::endl;
        else
            std::cout << "Failed! " << GetLastError() << std::endl;
        //printf("%i: %s - 0x%I64X\n", frames - i - 1, symbol->Name, symbol->Address);
    }

    free(line);
    free(symbol);
    SymCleanup(process);
}

void Blah()
{
    PrintStackWalk64(0);
    //PrintStackCpp(0);
    //PrintStack();
    //PrintStackTrace();
}

int main()
{
    Blah();
}
