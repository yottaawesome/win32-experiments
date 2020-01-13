#include <iostream>
#include <Windows.h>
#include <dbghelp.h>
#include "StackTracer.h"

#pragma comment(lib, "dbghelp.lib")

// From https://stackoverflow.com/questions/5693192/win32-backtrace-from-c-code
void printStack(void)
{
    unsigned int   i;
    void* stack[100];
    unsigned short frames;
    SYMBOL_INFO* symbol;
    HANDLE         process;

    process = GetCurrentProcess();

    SymInitialize(process, NULL, TRUE);

    frames = CaptureStackBackTrace(0, 100, stack, NULL);
    symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    if (symbol == nullptr)
        return;
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (i = 0; i < frames; i++)
    {
        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);

        printf("%i: %s - 0x%I64X\n", frames - i - 1, symbol->Name, symbol->Address);
    }

    free(symbol);
}

// Doesn't work
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
    PrintStackTrace();
}
