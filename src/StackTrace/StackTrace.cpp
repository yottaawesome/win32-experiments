#include <iostream>
#include <Windows.h>
#include <dbghelp.h>
#include "StackTracer.h"

#pragma comment(lib, "dbghelp.lib")

// From https://stackoverflow.com/questions/5693192/win32-backtrace-from-c-code
void PrintStack()
{
    constexpr int MaxFunctionNameLength = 255;
    void* stack[100];
    unsigned short frames = CaptureStackBackTrace(0, 100, stack, nullptr);

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, true);

    SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + (MaxFunctionNameLength + 1) * sizeof(char), 1);
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
            std::cout << "Failed!";
        //printf("%i: %s - 0x%I64X\n", frames - i - 1, symbol->Name, symbol->Address);
    }

    free(line);
    free(symbol);
}

// Doesn't work, probably due to this sample being built for x64.
// I tried adjusting it, but didn't have much luck.
// From https://stackoverflow.com/questions/22465253/symgetlinefromaddr-not-working-properly
/*
int LogStackTrace()
{
    void* stack[1024];
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    WORD numberOfFrames = CaptureStackBackTrace(0, 1000, stack, NULL);
    SYMBOL_INFO* symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO));
    symbol->MaxNameLen = 1024;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    IMAGEHLP_LINE64* line = (IMAGEHLP_LINE64*)malloc(sizeof(IMAGEHLP_LINE64));
    line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    printf("Caught exception ");
    for (int i = 0; i < numberOfFrames; i++)
    {
        SymFromAddr(process, (DWORD64)(stack[i]), NULL, symbol);
        DWORD dwDisplacement;
        SymGetLineFromAddr64(process, (DWORD)(stack[i]), &dwDisplacement, line);
        printf("at %s in %s, address 0x%0X\n", symbol->Name, line->FileName, symbol->Address);
    }
    return 0;
}
*/

int main()
{
    PrintStack();
    //PrintStackTrace();
}
