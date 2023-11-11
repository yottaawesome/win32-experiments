export module common;
import std;
import win32;

export namespace Common
{
    namespace WinSock = Win32::WinSock;

    std::string TranslateErrorCode(
        const Win32::DWORD errorCode,
        const std::wstring& moduleNameToLoad = {}
    )
    {
        Win32::HMODULE moduleToSearch = nullptr;
        if (not moduleNameToLoad.empty())
            moduleToSearch = Win32::LoadLibraryW(moduleNameToLoad.c_str());

        const Win32::DWORD flags =
            Win32::FormatMessageFlags::AllocateBuffer 
            bitor Win32::FormatMessageFlags::MessageFromSystem
            bitor Win32::FormatMessageFlags::IgnoreInserts 
            bitor (moduleToSearch ? Win32::FormatMessageFlags::FromHModule : 0);

        void* messageBuffer = nullptr;
        Win32::FormatMessageA(
            flags,
            moduleToSearch,
            errorCode,
            0,
            reinterpret_cast<char*>(&messageBuffer),
            0,
            nullptr
        );
        if (moduleToSearch)
            Win32::FreeLibrary(moduleToSearch);

        if (!messageBuffer)
        {
            const auto lastError = Win32::GetLastError();
            return std::format(
                "FormatMessageA() failed on code {} with error {}",
                errorCode,
                lastError
            );
        }

        std::string msg(static_cast<char*>(messageBuffer));
        Win32::LocalFree(messageBuffer);

        std::erase_if(
            msg,
            [](const char c) { return c == '\n' || c == '\r'; }
        );

        return msg;
    }

    struct Win32Error final
        : public std::runtime_error
    {
        Win32Error(const std::string& msg, const Win32::DWORD code)
            : std::runtime_error(
                std::format("{}: {}", msg, TranslateErrorCode(code)))
        { }
    };

    struct WinSockError final
        : public std::runtime_error
    {
        WinSockError(const std::string& msg, const Win32::DWORD code)
            : std::runtime_error(
                std::format("{}: {}", msg, TranslateErrorCode(code, L"ws2_32.dll")))
        { }
    };

    struct AddrInfoWDeleter final
    {
        void operator()(WinSock::ADDRINFOW* obj)
        {
            WinSock::FreeAddrInfoW(obj);
        }
    };
    using AddrInfoWUniquePtr =
        std::unique_ptr<WinSock::ADDRINFOW, AddrInfoWDeleter>;

    struct BasicSocket
    {
        ~BasicSocket()
        {
            if (Socket and Socket != WinSock::InvalidSocket)
                WinSock::closesocket(Socket);
        }

        BasicSocket(WinSock::SOCKET socket)
            : Socket(socket)
        {}

        WinSock::SOCKET Socket = WinSock::InvalidSocket;
    };

    void InitialiseWinsock()
    {
        WinSock::WSADATA wsaData = { 0 };
        const int result = WinSock::WSAStartup(
            Win32::MakeWord(2, 2),
            &wsaData
        );
        if (result != 0)
            throw std::runtime_error("WSAStartup failed");
    }

    struct WinSockScope
    {
        WinSockScope(const WinSockScope&) = delete;
        WinSockScope operator=(const WinSockScope&) = delete;

        ~WinSockScope()
        {
            WinSock::WSACleanup();
        }

        WinSockScope()
        {
            InitialiseWinsock();
        }
    };
}