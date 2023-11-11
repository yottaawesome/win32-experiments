import std;
import win32;

namespace Common
{
    namespace WinSock = Win32::WinSock;

    struct AddrInfoWDeleter final
    {
        void operator()(WinSock::ADDRINFOW* obj)
        {
            WinSock::FreeAddrInfoW(obj);
        }
    };
    using AddrInfoWUniquePtr =
        std::unique_ptr<WinSock::ADDRINFOW, AddrInfoWDeleter>;

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

        WinSockScope()
        {
            InitialiseWinsock();
        }
        ~WinSockScope()
        {
            WinSock::WSACleanup();
        }
    };
}

namespace DemoA
{
    namespace WinSock = Win32::WinSock;

    WinSock::ADDRINFOW* Resolve(const std::wstring& host, const unsigned short portNum)
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfow
        std::wstring portNumber;
        if (portNum)
            portNumber = std::to_wstring(portNum);

        WinSock::ADDRINFOW* addrResult;
        WinSock::ADDRINFOW hints
        {
            .ai_family = (int)WinSock::AddressFamily::Inet,
            .ai_socktype = (int)WinSock::SocketTypes::Stream,
            .ai_protocol = (int)WinSock::Protocol::TCP
        };
        const int status = WinSock::GetAddrInfoW(
            host.c_str(),
            portNumber.c_str(),
            &hints,
            &addrResult
        );
        if (status)
            throw std::runtime_error("GetAddrInfoW() failed");

        return addrResult;
    }

    void Run()
    {
        Common::WinSockScope scope;
        auto addr = Common::AddrInfoWUniquePtr(
            Resolve(L"www.microsoft.com", 80)
        );
    }
}

// See https://learn.microsoft.com/en-us/windows/win32/winsock/using-winsock
auto main() -> int
{
    DemoA::Run();
	return 0;
}
