// Based on https://learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
import std;

int main()
{
    constexpr auto BUFSIZE = 4096;
    char chBuf[BUFSIZE];
    
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdout == INVALID_HANDLE_VALUE || hStdin == INVALID_HANDLE_VALUE)
        ExitProcess(1);

    // Send something to this process's stdout using printf.
    std::wcout << "\n ** This is a message from the child process. ** \n";

    // This simple algorithm uses the existence of the pipes to control execution.
    // It relies on the pipe buffers to ensure that no data is lost.
    // Larger applications would use more advanced process control.

    for (;;)
    {
        DWORD dwRead;
        DWORD dwWritten;
        // Read from standard input and stop on error or no data.
        BOOL bSuccess = ReadFile(hStdin, chBuf, BUFSIZE, &dwRead, nullptr);
        if (!bSuccess || dwRead == 0)
            break;

        // Write to standard output and stop on error.
        bSuccess = WriteFile(hStdout, chBuf, dwRead, &dwWritten, nullptr);
        std::wcout << L"Hello" << std::endl;
        if (!bSuccess)
            break;
    }

    return 0;
}