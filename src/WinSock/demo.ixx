export module demo;
import std;
import win32;
import common;

export namespace DemoA
{
    namespace WinSock = Win32::WinSock;

    WinSock::ADDRINFOW* Resolve(const std::wstring& host, const unsigned short portNum)
    {
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
        // https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfow
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

        Common::BasicSocket socket = Common::Open(addr.get());
        Common::Assert(socket.Get() != WinSock::InvalidSocket, "Expected valid socket");
        Common::Bind(socket.Get(), L"", 27015);
    }
}