// https://learn.microsoft.com/en-us/windows/win32/winsock/complete-server-code
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

import std;
import shared;

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
            auto endpoint = AddrInfoUniquePtr{};
            if (int addrResult{ getaddrinfo(nullptr, DEFAULT_PORT, &hints, std::out_ptr(endpoint)) }; addrResult != 0)
			    throw WinSockError{ addrResult, "getaddrinfo failed" };
			return endpoint;
        }();

	// Set up a listening socket and accept a client connection. We only expect one connection, so we 
    // close the listening socket after accepting one client.
	auto clientSocket = 
        [addrInfo = std::move(addrInfo)] -> Socket
        {
            // Setup the TCP listening socket
            auto listenSocket = Socket{ socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol) };
            if (not listenSocket.IsValid())
                throw WinSockError{ WSAGetLastError(), "socket failed" };
            if (bind(listenSocket.handle, addrInfo->ai_addr, (int)addrInfo->ai_addrlen) == SOCKET_ERROR)
                throw WinSockError{ WSAGetLastError(), "bind failed" };
            if (listen(listenSocket.handle, SOMAXCONN) == SOCKET_ERROR)
                throw WinSockError{ WSAGetLastError(), "listen failed" };

            // Accept a client socket
            auto clientSocket = Socket{ accept(listenSocket.handle, nullptr, nullptr) };
            if (not clientSocket.IsValid())
                throw WinSockError{ WSAGetLastError(), "accept failed" };
            return clientSocket;
        }();

    // Receive until the peer shuts down the connection
    [clientSocket = std::move(clientSocket)] mutable
    {
        while (true)
        {
            if (auto result = clientSocket.Recv(); result)
            {
                // Echo the buffer back to the sender
                std::println("Bytes received: {}: {}", result->size(), *result);
                auto bytesSent = clientSocket.Send(*result);
                std::println("Bytes sent: {}. Closing connection.", bytesSent);
                clientSocket.Close();
                break;
            }
            else
            {
                std::println("Connection closed by peer.");
                break;
            }
        }
    }();

    return 0;
}
catch (const std::exception& ex)
{
    std::println("Exception: {}", ex.what());
    return 1;
}