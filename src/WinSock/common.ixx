export module common;
import std;
import std.compat;
import win32;

export namespace Common
{
    namespace WinSock = Win32::WinSock;

    inline constexpr bool IsDebug() noexcept
    {
#ifdef _DEBUG
        return true;
#else
        return false;
#endif
    }

    inline void Assert(
        const bool value,
        const std::string_view msg = "",
        const std::source_location& loc = std::source_location::current()
    )
    {
        if constexpr (IsDebug())
        {
            if (value)
                return;
            std::wcerr <<
                std::format(
                    "Assertion failed: {}\nSource:\n\tFunction: {}\n\tFile: {}:{}:{}",
                    msg,
                    loc.function_name(),
                    loc.file_name(),
                    loc.line(),
                    loc.column()
                ).c_str();
            std::abort();
        }
    }

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
            : m_code(code),
            std::runtime_error(
                std::format("{}: {}", msg, TranslateErrorCode(code)))
        { }

        DWORD ErrorCode() const noexcept
        {
            return m_code;
        }

        private:
            DWORD m_code = 0;
    };

    struct WinSockError final
        : public std::runtime_error
    {
        WinSockError(const std::string& msg, const Win32::DWORD code)
            : m_code(code), 
            std::runtime_error(
                std::format("{}: {}", msg, TranslateErrorCode(code, L"ws2_32.dll")))
        { }

        DWORD ErrorCode() const noexcept
        {
            return m_code;
        }

        private:
            DWORD m_code = 0;
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

    class BasicSocket final
    {
        public:
            ~BasicSocket()
            {
                Close();
            }

            BasicSocket(const BasicSocket&) = delete;
            BasicSocket& operator=(const BasicSocket&) = delete;

            BasicSocket(BasicSocket&& other) noexcept
            {
                Move(other);
            }
            BasicSocket& operator=(BasicSocket&& other) noexcept
            {
                Move(other);
                return *this;
            }

        public:
            BasicSocket(WinSock::SOCKET socket)
                : m_socket(socket)
            {}

        public:
            operator SOCKET() noexcept
            {
                return m_socket;
            }

        public:
            WinSock::SOCKET Get() noexcept
            {
                return m_socket;
            }

        private:
            WinSock::SOCKET m_socket = WinSock::InvalidSocket;

        private:
            void Move(BasicSocket& other)
            {
                Close();
                m_socket = other.m_socket;
                other.m_socket = WinSock::InvalidSocket;
            }

            void Close()
            {
                if (m_socket and m_socket != WinSock::InvalidSocket)
                    WinSock::closesocket(m_socket);
                m_socket = WinSock::InvalidSocket;
            }
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

    void Bind(
        const WinSock::SOCKET socket, 
        const std::optional<std::wstring>& interfaceAddrIp4, 
        const unsigned short localPort
    )
    {
        WinSock::IN_ADDR addr{ 0 };
        if (interfaceAddrIp4)
        {
            Win32::LPCWSTR terminator = nullptr;
            const Win32::LONG ntstatus = WinSock::RtlIpv4StringToAddressW(
                interfaceAddrIp4->c_str(),
                true,
                &terminator,
                &addr
            );
            if (Win32::HasFailed(ntstatus))
            {
                throw std::runtime_error(
                    "RtlIpv4StringToAddressW() failed"
                );
            }
        }

        WinSock::sockaddr_in bindAddress{ 
            .sin_family = (int)WinSock::AddressFamily::Inet,
            .sin_port = localPort ? WinSock::htons(localPort) : 0ui16,
            .sin_addr = addr
        };
        const int status = WinSock::bind(
            socket,
            (WinSock::sockaddr*)&bindAddress,
            sizeof(bindAddress)
        );
        if (status == WinSock::SocketError)
            throw WinSockError("bind() failed", WinSock::WSAGetLastError());
    }

    BasicSocket Open(const ADDRINFOW& addrResult)
    {
        SOCKET socket = WinSock::socket(
            addrResult.ai_family,
            addrResult.ai_socktype,
            addrResult.ai_protocol
        );
        if (socket == WinSock::InvalidSocket)
            throw WinSockError("socket() failed", WinSock::WSAGetLastError());

        return { socket };
    }

    void Connect(const SOCKET socket, const WinSock::ADDRINFOW& addr)
    {
        Assert(socket && socket != WinSock::InvalidSocket, "Invalid SOCKET value.");

        const int result = WinSock::connect(
            socket,
            addr.ai_addr,
            static_cast<int>(addr.ai_addrlen)
        );
        if (result == WinSock::SocketError)
            throw WinSockError("connect() failed", WinSock::WSAGetLastError());
    }
}