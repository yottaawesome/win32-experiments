#include <iostream>
#include <Windows.h>
#include <DbgHelp.h>

#pragma comment(lib, "Dbghelp.lib")

import CrashHandler;

// https://docs.microsoft.com/en-us/windows/win32/api/minidumpapiset/nf-minidumpapiset-minidumpwritedump
typedef bool(WINAPI* MINIDUMPWRITEDUMP)(
    HANDLE hProcess, 
    DWORD dwPid, 
    HANDLE hFile, 
    MINIDUMP_TYPE DumpType, 
    PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    PMINIDUMP_CALLBACK_INFORMATION CallbackParam
);

// https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-unhandledexceptionfilter
long WINAPI HandleException(_EXCEPTION_POINTERS* apExceptionInfo)
{
    // Based on https://stackoverflow.com/a/9020804/7448661
    HMODULE dbghelpLib = nullptr;
    HANDLE dumpFile = nullptr;
    do
    {
        dbghelpLib = LoadLibraryW(L"dbghelp.dll");
        if (dbghelpLib == nullptr)
            break;

        const MINIDUMPWRITEDUMP pMiniDumpWriteDump = (MINIDUMPWRITEDUMP)GetProcAddress(dbghelpLib, "MiniDumpWriteDump");
        if (pMiniDumpWriteDump == nullptr)
            break;

        dumpFile = CreateFileW(
            L"dump_name.dmp",
            GENERIC_WRITE,
            FILE_SHARE_WRITE,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );
        if (dumpFile == INVALID_HANDLE_VALUE)
            break;

        // https://docs.microsoft.com/en-us/windows/win32/api/minidumpapiset/ns-minidumpapiset-minidump_exception_information
        _MINIDUMP_EXCEPTION_INFORMATION exceptionInfo
        {
            .ThreadId = GetCurrentThreadId(),
            .ExceptionPointers = apExceptionInfo,
            .ClientPointers = false
        };
        pMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dumpFile, MiniDumpNormal, &exceptionInfo, nullptr, nullptr);
    } while (false);

    if (dumpFile && dumpFile != INVALID_HANDLE_VALUE)
        CloseHandle(dumpFile);
    if (dbghelpLib)
        FreeLibrary(dbghelpLib);

    return EXCEPTION_CONTINUE_SEARCH;
}

// See this excellent post on analyzing dumps: https://stackoverflow.com/questions/734272/how-to-use-windbg-to-analyze-the-crash-dump-for-vc-application
// Other resources
// https://docs.microsoft.com/en-us/visualstudio/debugger/using-dump-files?view=vs-2019
// https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/crash-dump-files
// https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/enabling-postmortem-debugging
// https://stackoverflow.com/questions/1649117/analysing-crash-dump-in-windbg
// http://windbg.info/doc/1-common-cmds.html
// https://stackoverflow.com/questions/30019889/how-to-set-up-symbols-in-windbg
// http://www.windbg.xyz/windbg/article/4-PDB-Symbol-File
// https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/getting-started-with-windbg
// https://stackoverflow.com/questions/27562203/load-dbg-symbol-file-into-windbg
// https://stackoverflow.com/questions/39230167/retrieving-the-stack-trace-from-the-stored-exception-context-in-a-minidump-simi
// (From above) >> "Dumps obtained from Windows Error Reporting typically have a useless current context set on the faulting thread, with a stack deep in WerpReportFault. The actual context at the time of the exception can be retrieved with .ecxr -- which also sets the context in such a way that subsequent commands on the same thread (such as k) return the "correct" information."
// From the dump generated, you can open it in WinDbg and type .ecxr to set the correct context

// It's also possible to create dumps using Windows Error Reporting (WER)
// You need to create a key HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\<application name>.exe
// e.g. HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\CrashDump.exe
// From there, you can control a few different options, such as the type of dump and number of dumps to keep
// See https://docs.microsoft.com/en-us/windows/win32/wer/collecting-user-mode-dumps

int main(int argc, char* args[])
{
    SetUnhandledExceptionFilter(HandleException);
    BoltDownExceptionFilter();
    // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-setunhandledexceptionfilter

    std::cout << "Triggering STATUS_ACCESS_VIOLATION...";
    char* a = nullptr;
    a[0] = 'b';

    return 0;
}
