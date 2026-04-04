// https://learn.microsoft.com/en-us/windows/win32/winsock/complete-client-code
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

import std;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

struct WsaContext
{
    ~WsaContext()
    {
        WSACleanup();
    }
    WsaContext()
    {
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0)
        {
            std::println("WSAStartup failed with error: {}\n", iResult);
            throw std::runtime_error("WSAStartup failed");
        }
    }
};

struct WinSockError : std::runtime_error
{
    const int code = 0;
    WinSockError(int code, std::string msg) : code(code), std::runtime_error(std::format("WinSock error {}: {}", code, std::move(msg)))
    {}
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

    constexpr operator bool(this const Socket& self) { return self.handle != INVALID_SOCKET; }

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

auto main(int argc, char** argv) -> int
try
{
    if (argc != 2)
    {
        std::println("usage: {} server-name", argv[0]);
		std::println("If server-name is not provided, it defaults to localhost.");
    }

    // Initialize Winsock
    auto ws = WsaContext{};

    // Resolve the server address and port
    auto result =
        [](const char* serverName) -> AddrInfoUniquePtr
        {
            struct addrinfo hints { 
                .ai_family = AF_UNSPEC,
                .ai_socktype = SOCK_STREAM,
                .ai_protocol = IPPROTO_TCP
            };
            auto result = AddrInfoUniquePtr{};
            constexpr auto defaultPort = "27915";
            if (auto addrResult = getaddrinfo(serverName, defaultPort, &hints, std::out_ptr(result)); addrResult != 0)
				throw WinSockError{ addrResult, "getaddrinfo failed" };
            return result;
		}(argc == 2 ? argv[1] : "localhost");

    // Attempt to connect to an address until one succeeds
    auto connectSocket = 
		[result = std::move(result)] -> Socket
        {
            for (auto ptr = result.get(); ptr != nullptr; ptr = ptr->ai_next)
            {
                // Create a SOCKET for connecting to server
                auto connectSocket = Socket{ socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol) };
                if (not connectSocket)
                    throw WinSockError{ WSAGetLastError(), "socket failed" };
                // Connect to server.
                if (connect(connectSocket.handle, ptr->ai_addr, (int)ptr->ai_addrlen) != SOCKET_ERROR)
                    return connectSocket;
            }
            throw std::runtime_error{ "Unable to connect to server!" };
        }();

    // Send an initial buffer
    [connectSocket = std::move(connectSocket)]
    {
        // We expect this to echo back from the server.
        auto bytesSent = connectSocket.Send("Hello, world!");
        std::println("Bytes Sent: {}", bytesSent);

        // Receive until the peer closes the connection
        while (true)
        {
            if (auto received = connectSocket.Recv(); received)
            {
                std::println("Bytes received: {}: {}", received->size(), *received);
            }
            else
            {
                std::println("Connection closed by peer.");
				break;
            }
        };
    }();

    return 0;
}
catch (const std::exception& e)
{
    std::println("Error: {}", e.what());
    return 1;
}
