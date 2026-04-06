// Note that this sample demonstrates overlapped (asynchronous) I/O, not true non-blocking sockets,
// which are set via ioctlsocket and require polling via select(), WSAPoll(), or WSAEventSelect(). 
// The project name is a misnomer.
// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsarecv
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
import std;

struct WinSockError : std::runtime_error
{
    const int code = 0;
    WinSockError(int code, std::string msg) : code(code), std::runtime_error(std::format("WinSock error {}: {}", code, std::move(msg)))
    {}
};

struct Overlapped : OVERLAPPED
{
    ~Overlapped()
    {
        if (hEvent)
        {
            WSACloseEvent(hEvent);
            hEvent = nullptr;
        }
    }

	Overlapped(const Overlapped&) = delete;
	Overlapped& operator=(const Overlapped&) = delete;

    Overlapped() : 
        OVERLAPPED{
            .hEvent = [] -> HANDLE
            {
                auto event = WSACreateEvent();
                return event ? event : throw std::runtime_error("Failed to create event");
            }()
        }
    { }
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
        if (int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); iResult != 0)
            throw WinSockError{ iResult, "WSAStartup failed" };
    }
	WsaContext(const WsaContext&) = delete;
	WsaContext& operator=(const WsaContext&) = delete;
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
};

struct AddrInfoDeleter
{
    constexpr AddrInfoDeleter() = default;
    static void operator()(addrinfo* p) { freeaddrinfo(p); }
};
using AddrInfoUniquePtr = std::unique_ptr<addrinfo, AddrInfoDeleter>;

int main(int argc, char** argv)
try
{
    if (argc != 2) 
    {
        std::println("usage: {} server-name", argv[0]);
        return 1;
    }
    auto wsaContext = WsaContext{};

    auto endpoint = 
		[address = argv[1]] -> AddrInfoUniquePtr
        {
            // Initialize the hints to retrieve the server address for IPv4
            auto hints = addrinfo{
                .ai_family = AF_INET,
                .ai_socktype = SOCK_STREAM,
                .ai_protocol = IPPROTO_TCP
            };
            auto endpoint = AddrInfoUniquePtr{};
            if (auto rc = getaddrinfo(address, "27015", &hints, std::out_ptr(endpoint)); rc != 0)
				throw WinSockError{ rc, "getaddrinfo failed" };
            return endpoint;
        }();

	auto connSocket = 
        [endpoint = std::move(endpoint)] -> Socket
        {
            for (auto ptr = endpoint.get(); ptr != nullptr; ptr = ptr->ai_next)
            {
                auto connSocket = Socket{ socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol) };
                if (not connSocket.IsValid())
					throw WinSockError{ WSAGetLastError(), "socket failed" };

                if (connect(connSocket.handle, ptr->ai_addr, (int)ptr->ai_addrlen) == 0)
                {
					std::println("Client connected...");
                    return connSocket;
                }
                if (auto err = WSAGetLastError(); err != WSAECONNREFUSED)
                    throw WinSockError{ err, "connect failed" };
            }
			throw std::runtime_error("Unable to connect to server!");
        }();

    return
        [connSocket = std::move(connSocket)]
        {
            // Create an event handle and setup an overlapped structure.
            auto recvOverlapped = Overlapped{};
            auto buffer = std::array<char, 4096>{};
            auto buffers = std::array{ 
                WSABUF{ 
                    .len = static_cast<ULONG>(buffer.size()), 
                    .buf = buffer.data() 
                }
            };
            // Call WSARecv until the peer closes the connection
            // or until an error occurs
            while (true)
            {
                auto flags = DWORD{};
                auto recvBytes = DWORD{};
				// WSASend and WSARecv can accept an array of WSABUFs to scatter/gather data.
                if (WSARecv(connSocket.handle, buffers.data(), static_cast<DWORD>(buffers.size()), &recvBytes, &flags, &recvOverlapped, nullptr) == SOCKET_ERROR)
                    if (auto err = WSAGetLastError(); err != WSA_IO_PENDING)
                        throw WinSockError{ err, "WSARecv failed" };
                if (WSAWaitForMultipleEvents(1, &recvOverlapped.hEvent, true, INFINITE, true) == WSA_WAIT_FAILED)
                    throw WinSockError{ WSAGetLastError(), "WSAWaitForMultipleEvents failed" };
                if (not WSAGetOverlappedResult(connSocket.handle, &recvOverlapped, &recvBytes, false, &flags))
                    throw WinSockError{ WSAGetLastError(), "WSARecv operation failed" };

                std::println("Read {} bytes", recvBytes);
                WSAResetEvent(recvOverlapped.hEvent);
                // If 0 bytes are received, the connection was closed
                if (recvBytes == 0)
                    break;
            }
            return 0;
	    }();
}
catch(const std::exception& ex)
{
    std::println("Exception: {}", ex.what());
    return 1;
}