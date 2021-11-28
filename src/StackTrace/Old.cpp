#include <iostream>
#include <Windows.h>
#include <dbghelp.h>

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