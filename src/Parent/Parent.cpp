// Based on https://learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
// To run this in Visual Studio, set Properties > Debugging > Command Arguments to file.txt

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>

constexpr int BUFSIZE = 4096;

void CreateChildProcess(
    const std::wstring& path,
    const std::wstring& cmdline,
    HANDLE hChildStd_OUT_Wr, 
    HANDLE hChildStd_IN_Rd
);
void WriteToPipe(HANDLE hInputFile, HANDLE hChildStd_IN_Wr);
void ReadFromPipe(HANDLE hChildStd_OUT_Rd);
void ErrorExit(const wchar_t*);

int main(int argc, char* argv[])
{
    // Get a handle to an input file for the parent. 
    // This example assumes a plain text file and uses string output to verify data flow. 
    if (argc == 1)
        ErrorExit(L"Please specify an input file.\n");

    HANDLE hChildStd_IN_Rd = nullptr;
    HANDLE hChildStd_IN_Wr = nullptr;
    HANDLE hChildStd_OUT_Rd = nullptr;
    HANDLE hChildStd_OUT_Wr = nullptr;
    HANDLE hInputFile = nullptr;

    std::cout << ("\n->Start of parent execution.\n");

    // Set the bInheritHandle flag so pipe handles are inherited.
    SECURITY_ATTRIBUTES saAttr{
        .nLength = sizeof(SECURITY_ATTRIBUTES),
        .lpSecurityDescriptor = nullptr,
        .bInheritHandle = true
    };

    // Create a pipe for the child process's STDOUT. 
    if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0))
        ErrorExit(L"StdoutRd CreatePipe");

    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
        ErrorExit(L"Stdout SetHandleInformation");

    // Create a pipe for the child process's STDIN. 
    if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0))
        ErrorExit(L"Stdin CreatePipe");

    // Ensure the write handle to the pipe for STDIN is not inherited. 
    if (!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
        ErrorExit(L"Stdin SetHandleInformation");

    // Create the child process. 
    CreateChildProcess({}, L"child", hChildStd_OUT_Wr, hChildStd_IN_Rd);

    hInputFile = CreateFileA(
        argv[1],
        GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_READONLY,
        nullptr
    );
    if (hInputFile == INVALID_HANDLE_VALUE)
        ErrorExit(L"CreateFile");

    // Write to the pipe that is the standard input for a child process. 
    // Data is written to the pipe's buffers, so it is not necessary to wait
    // until the child process is running before writing data.
    WriteToPipe(hInputFile, hChildStd_IN_Wr);
    std::cout << std::format("\n->Contents of {} written to child STDIN pipe.\n", argv[1]);

    // Read from pipe that is the standard output for child process. 
    std::cout << ("\n->Contents of child process STDOUT:\n\n");
    ReadFromPipe(hChildStd_OUT_Rd);

    std::cout << ("\n->End of parent execution.\n");

    // The remaining open handles are cleaned up when this process terminates. 
    // To avoid resource leaks in a larger application, close handles explicitly. 
    return 0;
}

// Create a child process that uses the previously created pipes for STDIN and STDOUT.
void CreateChildProcess(
    const std::wstring& path,
    const std::wstring& cmdline,
    HANDLE hChildStd_OUT_Wr,
    HANDLE hChildStd_IN_Rd
)
{
    PROCESS_INFORMATION piProcInfo{ 0 };
    STARTUPINFO siStartInfo{
        .cb = sizeof(STARTUPINFO),
        .dwFlags = STARTF_USESTDHANDLES,
        .hStdInput = hChildStd_IN_Rd,
        .hStdOutput = hChildStd_OUT_Wr,
        .hStdError = hChildStd_OUT_Wr,
    };
    bool bSuccess = false;

    // Create the child process. 
    bSuccess = CreateProcess(
        path.empty() ? nullptr : const_cast<wchar_t*>(path.c_str()),
        const_cast<wchar_t*>(cmdline.c_str()),     // command line 
        nullptr,          // process security attributes 
        nullptr,          // primary thread security attributes 
        true,          // handles are inherited 
        0,             // creation flags 
        nullptr,          // use parent's environment 
        nullptr,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo     // receives PROCESS_INFORMATION 
    );

    // If an error occurs, exit the application. 
    if (!bSuccess)
        ErrorExit(L"CreateProcess");

    // Close handles to the child process and its primary thread.
    // Some applications might keep these handles to monitor the status
    // of the child process, for example. 
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    // Close handles to the stdin and stdout pipes no longer needed by the child process.
    // If they are not explicitly closed, there is no way to recognize that the child process has ended.
    CloseHandle(hChildStd_OUT_Wr);
    CloseHandle(hChildStd_IN_Rd);
}

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data. 
void WriteToPipe(
    HANDLE hInputFile,
    HANDLE hChildStd_IN_Wr
)
{
    DWORD dwRead; 
    DWORD dwWritten;
    char chBuf[BUFSIZE];
    bool bSuccess = false;

    for (;;)
    {
        bSuccess = ReadFile(hInputFile, chBuf, BUFSIZE, &dwRead, nullptr);
        if (!bSuccess || dwRead == 0) 
            break;

        bSuccess = WriteFile(hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, nullptr);
        if (!bSuccess) 
            break;
    }

    // Close the pipe handle so the child process stops reading. 
    if (!CloseHandle(hChildStd_IN_Wr))
        ErrorExit(L"StdInWr CloseHandle");
}

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data.
void ReadFromPipe(HANDLE hChildStd_OUT_Rd) 
{
    DWORD dwRead, dwWritten;
    char chBuf[BUFSIZE];
    bool bSuccess = false;
    HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    for (;;)
    {
        bSuccess = ReadFile(hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, nullptr);
        if (!bSuccess || dwRead == 0) 
            break;

        bSuccess = WriteFile(
            hParentStdOut, 
            chBuf,
            dwRead, 
            &dwWritten, 
            nullptr
        );
        if (!bSuccess) 
            break;
    }
}

// Format a readable error message, display a message box, 
// and exit from the application.
void ErrorExit(const wchar_t* lpszFunction)
{
    DWORD dw = GetLastError();

    void* lpMsgBuf;
    DWORD charCount = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        dw,
        0,
        (LPWSTR)&lpMsgBuf,
        0, 
        nullptr
    );

    std::wstring error;
    if (charCount)
    {
        error = std::format(L"{} failed with error {}: {}", lpszFunction, dw, (const wchar_t*)lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
    else 
        error = L"An unknown error occurred";

    MessageBox(nullptr, error.c_str(), L"Error", MB_OK);

    ExitProcess(1);
}