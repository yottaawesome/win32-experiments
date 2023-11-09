#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
import std;

namespace Strings
{
    // trim from start (in place)
    void LeftTrim(std::string& s)
    {
        s.erase(
            s.begin(),
            std::find_if(
                s.begin(),
                s.end(),
                [](int ch) { return !std::isspace(ch); }
            )
        );
    }

    // trim from end (in place)
    void RightTrim(std::string& s)
    {
        s.erase(
            std::find_if(
                s.rbegin(),
                s.rend(),
                [](int ch) { return !std::isspace(ch); }
            ).base(),
            s.end()
        );
    }

    // trim from both ends (in place)
    void Trim(std::string& s)
    {
        LeftTrim(s);
        RightTrim(s);
    }

    std::vector<std::string> Split(
        const std::string& stringToSplit,
        const std::string& delimiter
    )
    {
        size_t position = 0;
        // If we don't find it at all, add the whole string
        if (stringToSplit.find(delimiter, position) == std::string::npos)
            return { stringToSplit };

        std::vector<std::string> results;
        std::string intermediateString = stringToSplit;
        while ((position = intermediateString.find(delimiter)) != std::string::npos)
        {
            // split and add to the results
            std::string split = intermediateString.substr(0, position);
            results.push_back(split);

            // move up our position
            position += delimiter.length();
            intermediateString = intermediateString.substr(position);

            // On the last iteration, enter the remainder
            if (intermediateString.find(delimiter) == std::string::npos)
                results.push_back(intermediateString);
        }

        return results;
    }
}

namespace DemoA
{
    struct HandlerDeleter
    {
        void operator()(HANDLE h) { CloseHandle(h); }
    };
    using HandleUniquePtr = std::unique_ptr < std::remove_pointer_t<HANDLE>, HandlerDeleter>;

    constexpr int BUFSIZE = 4096;

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

    std::string ReadFromPipe(HANDLE hChildStd_OUT_Rd)
    {
        DWORD dwRead;
        char chBuf[BUFSIZE];
        bool bSuccess = false;
        HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

        std::string result;
        for (;;)
        {
            bSuccess = ReadFile(hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, nullptr);
            if (!bSuccess || dwRead == 0)
                break;

            result += std::string(chBuf, dwRead);
        }
        //Comes out as "SerialNumber  \r\r\n{serialnumber} \r\r\n\r\r\n"
        //std::cout << result;
        return result;
    }

    PROCESS_INFORMATION CreateChildProcess(
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

        // Close handles to the stdin and stdout pipes no longer needed by the child process.
        // If they are not explicitly closed, there is no way to recognize that the child process has ended.
        CloseHandle(hChildStd_OUT_Wr);
        CloseHandle(hChildStd_IN_Rd);

        return piProcInfo;
    }

    std::string Parse(const std::string& cmd)
    {
        std::vector tokens = Strings::Split(cmd, "\r\r\n");
        bool nextLine = false;
        std::string serialNumber;
        for (const std::string token : tokens)
        {
            if (nextLine)
            {
                serialNumber = token;
                break;
            }
            nextLine = token.starts_with("SerialNumber");
        }

        // Seems to come with two extra spaces at the end
        if (serialNumber.size() > 2)
        {
            serialNumber.pop_back();
            serialNumber.pop_back();
        }

        Strings::Trim(serialNumber);
        return serialNumber;
    }

    template<auto FParser = nullptr>
    auto Exec(const std::wstring& cmd)
    {
        // Get a handle to an input file for the parent. 
        // This example assumes a plain text file and uses string output to verify data flow. 
        if (cmd.empty())
            ErrorExit(L"Please specify a cmd command.\n");

        HANDLE hChildStd_IN_Rd = nullptr;
        HANDLE hChildStd_IN_Wr = nullptr;
        HANDLE hChildStd_OUT_Rd = nullptr;
        HANDLE hChildStd_OUT_Wr = nullptr;
        HANDLE hInputFile = nullptr;

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
        HandleUniquePtr ptrChildStdOutRd = HandleUniquePtr(hChildStd_OUT_Rd);

        // Create a pipe for the child process's STDIN. 
        if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0))
            ErrorExit(L"Stdin CreatePipe");
        // Ensure the write handle to the pipe for STDIN is not inherited. 
        if (!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
            ErrorExit(L"Stdin SetHandleInformation");
        HandleUniquePtr ptrChildStdInWr = HandleUniquePtr(hChildStd_IN_Wr);

        // Create the child process. 
        PROCESS_INFORMATION childProc = CreateChildProcess(
            LR"(C:\Windows\System32\cmd.exe)",
            cmd,
            hChildStd_OUT_Wr,
            hChildStd_IN_Rd
        );
        HandleUniquePtr ptrChildProcess = HandleUniquePtr(childProc.hProcess);
        HandleUniquePtr ptrChildThread = HandleUniquePtr(childProc.hThread);

        if constexpr (not std::is_null_pointer_v<decltype(FParser)>)
        {
            // Read from pipe that is the standard output for child process. 
            std::string output = ReadFromPipe(hChildStd_OUT_Rd);
            return Parse(output);
        }
    }

    template<auto FParser = nullptr>
    struct CmdRunner
    {
        auto operator()(const std::wstring& cmd) const
        {
            return Exec<FParser>(cmd);
        }
    };

    constexpr CmdRunner<Parse> GetBIOS;
}

int main()
{
    std::string serialNumber = DemoA::GetBIOS(LR"(C:\Windows\System32\cmd.exe /c wmic bios get serialnumber)");
    std::cout << std::format("Your BIOS serial number is \"{}\".\n", serialNumber);

    return 0;
}
