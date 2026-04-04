// https://learn.microsoft.com/en-us/windows/win32/winsock/complete-server-code
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
import std;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

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
    SOCKET handle;
    Socket(SOCKET s) : handle(s) {}
    ~Socket()
    {
        Close();
    }

    auto Close() -> void
    {
        if (handle == INVALID_SOCKET)
            return;
		shutdown(handle, SD_BOTH);
        closesocket(handle);
        handle = INVALID_SOCKET;
    }

	constexpr operator bool() const { return handle != INVALID_SOCKET; }

    // Move-only
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept : handle(other.handle) { other.handle = INVALID_SOCKET; }
    Socket& operator=(Socket&& other) noexcept
    {
        if (this == &other)
            return *this;
        Close();
        handle = other.handle;
        other.handle = INVALID_SOCKET;
        return *this;
	}
};

struct AddrInfoDeleter
{
	constexpr AddrInfoDeleter() = default;
    static void operator()(addrinfo* p) { freeaddrinfo(p); }
};
using AddrInfoUniquePtr = std::unique_ptr<addrinfo, AddrInfoDeleter>;

auto main() -> int
try
{
    // Initialize Winsock
    auto ws = WsaContext{};

    // Resolve the server address and port
    auto addrInfo = 
        [] -> AddrInfoUniquePtr
        {
            constexpr auto DEFAULT_PORT = "27915";
            auto hints = addrinfo{
                .ai_flags = AI_PASSIVE,
                .ai_family = AF_INET,
                .ai_socktype = SOCK_STREAM,
                .ai_protocol = IPPROTO_TCP,
            };
            auto result = AddrInfoUniquePtr{};
            if (int addrResult{ getaddrinfo(nullptr, DEFAULT_PORT, &hints, std::out_ptr(result)) }; addrResult != 0)
			    throw WinSockError{ addrResult, "getaddrinfo failed" };
			return result;
        }();

	// Set up a listening socket and accept a client connection. We only expect one connection, so we 
    // close the listening socket after accepting one client.
	auto clientSocket = 
        [addrInfo = std::move(addrInfo)] -> Socket
        {
            // Setup the TCP listening socket
            auto listenSocket = Socket{ socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol) };
            if (not listenSocket)
                throw WinSockError{ WSAGetLastError(), "socket failed" };
            if (bind(listenSocket.handle, addrInfo->ai_addr, (int)addrInfo->ai_addrlen) == SOCKET_ERROR)
                throw WinSockError{ WSAGetLastError(), "bind failed" };
            if (listen(listenSocket.handle, SOMAXCONN) == SOCKET_ERROR)
                throw WinSockError{ WSAGetLastError(), "listen failed" };

            // Accept a client socket
            auto clientSocket = Socket{ accept(listenSocket.handle, nullptr, nullptr) };
            if (not clientSocket)
                throw WinSockError{ WSAGetLastError(), "accept failed" };
            return clientSocket;
        }();

    // Receive until the peer shuts down the connection
    [clientSocket = std::move(clientSocket)] mutable
    {
        auto receiveBuffer = std::array<char, 512>{};
        auto result = int{};
        do
        {
            result = recv(clientSocket.handle, receiveBuffer.data(), static_cast<int>(receiveBuffer.size()), 0);
            if (result > 0)
            {
                std::println("Bytes received: {}: {}", result, std::string_view(receiveBuffer.data(), result));
                // Echo the buffer back to the sender
                auto bytesSent = int{ send(clientSocket.handle, receiveBuffer.data(), result, 0) };
                if (bytesSent == SOCKET_ERROR)
                    throw WinSockError{ WSAGetLastError(), "send failed" };
                // echo complete, shutdown
                std::println("Bytes sent: {}. Closing connection.", bytesSent);
				clientSocket.Close();
                break;
            }
            else if (result == 0)
            {
                std::println("Connection closing...");
            }
            else
            {
                throw WinSockError{ WSAGetLastError(), "recv failed" };
            }
        } while (result > 0);
    }();

    return 0;
}
catch (const std::exception& ex)
{
    std::println("Exception: {}", ex.what());
    return 1;
}