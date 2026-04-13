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
        const int Code = 0;
        WinSockError(int code, std::string msg) : Code(code), std::runtime_error(std::format("WinSock error {}: {}", Code, std::move(msg)))
        {}
    };

    struct Win32Error : std::runtime_error
    {
        const DWORD Code = 0;
        Win32Error(DWORD code, std::string msg) : Code(code), std::runtime_error(std::format("Win32 error {}: {}", Code, std::move(msg)))
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

		explicit constexpr operator bool() const { return IsValid(); }

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

        auto SetBlocking(this const Socket& self, bool blocking) -> void
        {
            auto mode = blocking ? 0ul : 1ul;
            if (ioctlsocket(self.handle, FIONBIO, &mode) == SOCKET_ERROR)
                throw WinSockError{ WSAGetLastError(), "ioctlsocket failed" };
		}

        struct [[nodiscard]] AsyncReceiveResult : OVERLAPPED
        {
            ~AsyncReceiveResult()
            {
                if (hEvent)
                {
                    CloseHandle(hEvent);
					hEvent = nullptr;
                }
			}
            AsyncReceiveResult() : OVERLAPPED{ .hEvent = CreateEventW(nullptr, true, false, nullptr) }
            {
                if (not hEvent)
                    throw Win32Error{ GetLastError(), "Failed to create event" };
            }
            AsyncReceiveResult(const AsyncReceiveResult&) = delete;
            auto operator=(const AsyncReceiveResult&) = delete;

            DWORD ErrorCode = 0;
            DWORD BytesTransferred = 0;
			std::string Buffer = std::string(512, '\0');

            auto WaitForCompletion(std::chrono::milliseconds wait = std::chrono::milliseconds{INFINITE}) -> bool
            {
                auto waitResult = WaitForSingleObject(hEvent, static_cast<DWORD>(wait.count()));
                if (waitResult == WAIT_OBJECT_0)
                    return true;
				if (waitResult == WAIT_TIMEOUT)
                    return false;
                throw Win32Error{ GetLastError(), "WaitForSingleObject failed" };
			}
        };
        auto AsyncRecv(this const Socket& self, std::uint32_t bufferSize) -> std::unique_ptr<AsyncReceiveResult>
        {
            auto result = std::make_unique<AsyncReceiveResult>();
            auto toRecv = std::array{
                WSABUF{
                    .len = static_cast<ULONG>(result->Buffer.size()),
					.buf = result->Buffer.data()
                }
            };
            auto status = WSARecv(
                self.handle,
                toRecv.data(),
                static_cast<DWORD>(toRecv.size()),
                nullptr,
                nullptr,
                result.get(),
                [](DWORD error, DWORD transferred, OVERLAPPED* overlapped, DWORD flags)
                {
                    auto* recvResult = static_cast<AsyncReceiveResult*>(overlapped);
                    if (error != 0)
                    {
                        recvResult->ErrorCode = error;
                        recvResult->Buffer = {};
                    }
                    else
                    {
                        recvResult->BytesTransferred = transferred;
						recvResult->Buffer.resize(transferred);
                    }
                }
            );
			return result;
        }

        // You can also set OVERLAPPED as the first member, then reinterpret_cast to get the AsyncSendResult in the completion callback.
        // That works because the OVERLAPPED is the first member, so the pointer to AsyncSendResult and the pointer to OVERLAPPED will be the same.
        struct [[nodiscard]] AsyncSendResult : OVERLAPPED
        {
            ~AsyncSendResult()
            {
                if (hEvent)
                {
                    CloseHandle(hEvent);
                    hEvent = nullptr;
                }
            }
            AsyncSendResult() : OVERLAPPED{ .hEvent = CreateEventW(nullptr, true, false, nullptr) }
            {
                if (not hEvent)
                    throw Win32Error{ GetLastError(), "Failed to create event" };
            }
            AsyncSendResult(const AsyncSendResult&) = delete;
            auto operator=(const AsyncSendResult&) = delete;

            DWORD ErrorCode = 0;
            DWORD BytesTransferred = 0;
            std::string Buffer;
            
            auto WaitForCompletion(std::chrono::milliseconds wait = std::chrono::milliseconds{ INFINITE }) -> bool
            {
                auto waitResult = WaitForSingleObject(hEvent, static_cast<DWORD>(wait.count()));
                if (waitResult == WAIT_OBJECT_0)
                    return true;
                if (waitResult == WAIT_TIMEOUT)
                    return false;
                throw Win32Error{ GetLastError(), "WaitForSingleObject failed" };
            }
        };
        auto AsyncSend(this const Socket& self, std::string_view data) -> std::unique_ptr<AsyncSendResult>
        {
            auto result = std::make_unique<AsyncSendResult>();
            result->Buffer = std::string(data);
            auto toSend = std::array{
                WSABUF{
                    .len = static_cast<ULONG>(result->Buffer.size()), 
                    .buf = result->Buffer.data() 
                }
            };

            auto status = WSASend(
                self.handle,
                toSend.data(),
                static_cast<DWORD>(toSend.size()),
                nullptr,
                0,
                result.get(),
                [](DWORD error, DWORD transferred, OVERLAPPED* overlapped, DWORD flags)
                {
                    auto* sendResult = static_cast<AsyncSendResult*>(overlapped);
                    if (error != 0)
						sendResult->ErrorCode = error;
                    else
                        sendResult->BytesTransferred = transferred;
                }
            );
            if (status == SOCKET_ERROR)
                if (auto err = WSAGetLastError(); err != WSA_IO_PENDING)
                    throw WinSockError{ err, "WSASend failed" };
			return result;
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