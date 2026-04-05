module;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

export module shared;
import std;

export
{
    struct WinSockError : std::runtime_error
    {
        const int code = 0;
        WinSockError(int code, std::string msg) : code(code), std::runtime_error(std::format("WinSock error {}: {}", code, std::move(msg)))
        {}
    };

    struct WsaContext
    {
        ~WsaContext()
        {
            WSACleanup();
        }
		WsaContext(const WsaContext&) = delete;
		WsaContext& operator=(const WsaContext&) = delete;
        WsaContext()
        {
            WSADATA wsaData;
            int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (iResult != 0)
                throw WinSockError{ iResult, "WSAStartup failed" };
        }
    };

    struct Socket
    {
        SOCKET handle = INVALID_SOCKET;
        Socket() = default;
        Socket(SOCKET s) : handle(s) {}
        ~Socket()
        {
            Close();
        }

        auto Close(this Socket& self) -> void
        {
            if (self.handle == INVALID_SOCKET)
                return;
            shutdown(self.handle, SD_BOTH);
            closesocket(self.handle);
            self.handle = INVALID_SOCKET;
        }

        constexpr auto IsValid(this const Socket& self) { return self.handle != INVALID_SOCKET; }

        // Move-only
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;
        Socket(Socket&& other) noexcept : handle(other.handle) { other.handle = INVALID_SOCKET; }
        auto operator=(this Socket& self, Socket&& other) noexcept -> Socket&
        {
            if (&self == &other)
                return self;
            self.Close();
            self.handle = other.handle;
            other.handle = INVALID_SOCKET;
            return self;
        }

        auto Send(this const Socket& self, const std::string_view& data) -> std::uint32_t
        {
            auto bytesSent = int{ send(self.handle, data.data(), static_cast<int>(data.size()), 0) };
            if (bytesSent == SOCKET_ERROR)
                throw WinSockError{ WSAGetLastError(), "send failed" };
            return bytesSent;
        }

        auto Recv(this const Socket& self) -> std::optional<std::string>
        {
            // You would need length prefixed messages or some other protocol to know how much to recv. 
            // For simplicity, we just recv into a fixed size buffer and return what we get.
            auto buffer = std::string(512, '\0');
            auto bytesReceived = int{ recv(self.handle, buffer.data(), static_cast<int>(buffer.size()), 0) };
            if (bytesReceived == SOCKET_ERROR)
                throw WinSockError{ WSAGetLastError(), "recv failed" };
            if (bytesReceived == 0) // closed
                return std::nullopt;
            buffer.resize(bytesReceived);
            return buffer;
        }
    };

    struct AddrInfoDeleter
    {
        constexpr AddrInfoDeleter() = default;
        static void operator()(addrinfo* p) { freeaddrinfo(p); }
    };
    using AddrInfoUniquePtr = std::unique_ptr<addrinfo, AddrInfoDeleter>;
}