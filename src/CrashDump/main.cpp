#include <iostream>
#include <Windows.h>
#include <DbgHelp.h>

#pragma comment(lib, "Dbghelp.lib")

// https://docs.microsoft.com/en-us/windows/win32/api/minidumpapiset/nf-minidumpapiset-minidumpwritedump
typedef bool(WINAPI* MINIDUMPWRITEDUMP)(
    HANDLE hProcess, 
    DWORD dwPid, 
    HANDLE hFile, 
    MINIDUMP_TYPE DumpType, 
    const PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    const PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    const PMINIDUMP_CALLBACK_INFORMATION CallbackParam
);

// https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-unhandledexceptionfilter
LONG WINAPI HandleException(struct _EXCEPTION_POINTERS* apExceptionInfo)
{
    // Based on https://stackoverflow.com/a/9020804/7448661
    HMODULE mhLib = LoadLibraryW(L"dbghelp.dll");
    MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(mhLib, "MiniDumpWriteDump");

    HANDLE hFile = CreateFileW(
        L"dump_name.dmp", 
        GENERIC_WRITE, 
        FILE_SHARE_WRITE, 
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, 
        nullptr
    );

    _MINIDUMP_EXCEPTION_INFORMATION ExInfo;
    ExInfo.ThreadId = ::GetCurrentThreadId();
    ExInfo.ExceptionPointers = apExceptionInfo;
    ExInfo.ClientPointers = FALSE;

    pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, nullptr, nullptr);
    CloseHandle(hFile);
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

int main(int argc, char* args[])
{
    // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-setunhandledexceptionfilter
    SetUnhandledExceptionFilter(HandleException);

    std::cout << "Triggering exception" << std::endl;

    char* a = nullptr;
    a[0] = 'b';

    return 0;
}
