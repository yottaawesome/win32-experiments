#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

import std;

using namespace std;

// --- Shared Infrastructure ---

// Coroutine Task type
struct Task {
    struct promise_type {
        Task get_return_object() { return {}; }
        suspend_never initial_suspend() { return {}; }
        suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {
            try {
                rethrow_exception(current_exception());
            } catch (const exception& e) {
                println("Caught exception: {}", e.what());
            }
            exit(1);
        }
    };
};

struct IoResult {
    DWORD bytes = 0;
    int error = 0;
};

// Awaitable for Winsock Overlapped I/O
struct IoAwaitable : OVERLAPPED {
    coroutine_handle<> handle;
    IoResult result;

    IoAwaitable() {
        Internal = InternalHigh = Offset = OffsetHigh = 0;
        hEvent = NULL;
    }

    bool await_ready() { return false; }
    void await_suspend(coroutine_handle<> h) { handle = h; }
    IoResult await_resume() { return result; }
};

// Global IOCP handle and extension functions
HANDLE g_iocp = INVALID_HANDLE_VALUE;
LPFN_ACCEPTEX lpfnAcceptEx = nullptr;
LPFN_CONNECTEX lpfnConnectEx = nullptr;
atomic<bool> g_stop = false;

void RegisterSocket(SOCKET s) {
    if (CreateIoCompletionPort((HANDLE)s, g_iocp, 0, 0) == NULL) {
        println("Failed to register socket with IOCP: {}", GetLastError());
    }
}

// --- Server Namespace ---

namespace Server {
    Task HandleClient(SOCKET s) {
        char buffer[1024];
        while (true) {
            IoAwaitable recvOp;
            WSABUF wsaBuf = { sizeof(buffer), buffer };
            DWORD flags = 0;
            
            // Start async receive
            if (WSARecv(s, &wsaBuf, 1, NULL, &flags, &recvOp, NULL) == SOCKET_ERROR) {
                if (WSAGetLastError() != WSA_IO_PENDING) break;
            }
            
            auto [bytes, err] = co_await recvOp;
            if (bytes == 0 || err != 0) break; // Connection closed or error

            // Echo data back
            IoAwaitable sendOp;
            wsaBuf.len = bytes;
            if (WSASend(s, &wsaBuf, 1, NULL, 0, &sendOp, NULL) == SOCKET_ERROR) {
                if (WSAGetLastError() != WSA_IO_PENDING) break;
            }
            
            co_await sendOp;
        }
        closesocket(s);
    }

    Task Run() {
        SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSock == INVALID_SOCKET) {
            println("Server: Failed to create socket: {}", WSAGetLastError());
            co_return;
        }
        RegisterSocket(listenSock);

        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(12345);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (::bind(listenSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            println("Server: Bind failed: {}", WSAGetLastError());
            co_return;
        }

        if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR) {
            println("Server: Listen failed: {}", WSAGetLastError());
            co_return;
        }

        println("Server: Listening on port 12345...");

        while (!g_stop) {
            SOCKET acceptSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            RegisterSocket(acceptSock);

            IoAwaitable acceptOp;
            char buf[(sizeof(sockaddr_in) + 16) * 2];
            DWORD received = 0;

            // Start async accept
            if (!lpfnAcceptEx(listenSock, acceptSock, buf, 0, 
                             sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, 
                             &received, &acceptOp)) {
                if (WSAGetLastError() != WSA_IO_PENDING) {
                    println("Server: AcceptEx failed: {}", WSAGetLastError());
                    closesocket(acceptSock);
                    break;
                }
            }
            
            co_await acceptOp;
            
            if (g_stop) {
                closesocket(acceptSock);
                break;
            }

            // Update socket context to enable inherited properties
            setsockopt(acceptSock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&listenSock, sizeof(listenSock));
            
            println("Server: Client connected.");
            HandleClient(acceptSock);
        }
        closesocket(listenSock);
        co_return;
    }
}

// --- Client Namespace ---

namespace Client {
    Task Run() {
        // Wait for server to start
        this_thread::sleep_for(1s);

        SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET) {
            println("Client: Failed to create socket: {}", WSAGetLastError());
            co_return;
        }
        RegisterSocket(s);

        // ConnectEx requires the socket to be bound first
        sockaddr_in localAddr = { AF_INET, 0, {{0}} };
        ::bind(s, (sockaddr*)&localAddr, sizeof(localAddr));

        sockaddr_in serverAddr = {};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(12345);
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

        println("Client: Connecting to server...");
        IoAwaitable connectOp;
        if (!lpfnConnectEx(s, (sockaddr*)&serverAddr, sizeof(serverAddr), NULL, 0, NULL, &connectOp)) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                println("Client: ConnectEx failed: {}", WSAGetLastError());
                closesocket(s);
                co_return;
            }
        }
        
        auto [cBytes, cErr] = co_await connectOp;
        if (cErr != 0) {
            println("Client: Connection failed with error: {}", cErr);
            closesocket(s);
            co_return;
        }
        
        setsockopt(s, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
        println("Client: Connected successfully!");

        string message = "Hello from C++23 Coroutine Client!";
        IoAwaitable sendOp;
        WSABUF wsaBuf = { (ULONG)message.size(), message.data() };
        
        println("Client: Sending: \"{}\"", message);
        if (WSASend(s, &wsaBuf, 1, NULL, 0, &sendOp, NULL) == SOCKET_ERROR) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                closesocket(s);
                co_return;
            }
        }
        co_await sendOp;

        char recvBuf[1024] = {0};
        IoAwaitable recvOp;
        wsaBuf = { sizeof(recvBuf), recvBuf };
        DWORD flags = 0;
        if (WSARecv(s, &wsaBuf, 1, NULL, &flags, &recvOp, NULL) == SOCKET_ERROR) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                closesocket(s);
                co_return;
            }
        }
        
        auto [bytes, err] = co_await recvOp;
        if (err == 0) {
            println("Client: Received echo: \"{}\"", string_view(recvBuf, bytes));
        } else {
            println("Client: Receive error: {}", err);
        }

        closesocket(s);
        println("Client: Finished.");
        
        this_thread::sleep_for(1s);
        g_stop = true;
        co_return;
    }
}

// --- Main ---

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        println("WSAStartup failed.");
        return 1;
    }

    g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (g_iocp == NULL) {
        println("Failed to create IOCP: {}", GetLastError());
        return 1;
    }

    // Retrieve extension function pointers
    SOCKET dummy = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    DWORD dwBytes = 0;
    GUID guidAcceptEx = WSAID_ACCEPTEX;
    WSAIoctl(dummy, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx), 
             &lpfnAcceptEx, sizeof(lpfnAcceptEx), &dwBytes, NULL, NULL);
    
    GUID guidConnectEx = WSAID_CONNECTEX;
    WSAIoctl(dummy, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidConnectEx, sizeof(guidConnectEx), 
             &lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, NULL, NULL);
    closesocket(dummy);

    if (!lpfnAcceptEx || !lpfnConnectEx) {
        println("Failed to get Winsock extension functions.");
        return 1;
    }

    // Start Server and Client coroutines
    Server::Run();
    Client::Run();

    // IOCP Event Loop
    println("Main: Entering event loop...");
    while (!g_stop) {
        DWORD bytesTransferred = 0;
        ULONG_PTR key = 0;
        LPOVERLAPPED overlapped = nullptr;
        
        // Wait for a completion packet
        if (GetQueuedCompletionStatus(g_iocp, &bytesTransferred, &key, &overlapped, 100)) {
            if (overlapped) {
                auto awaitable = static_cast<IoAwaitable*>(overlapped);
                awaitable->result = { bytesTransferred, 0 };
                awaitable->handle.resume();
            }
        } else {
            DWORD err = GetLastError();
            if (err == WAIT_TIMEOUT) {
                continue;
            }
            if (overlapped) {
                auto awaitable = static_cast<IoAwaitable*>(overlapped);
                awaitable->result = { bytesTransferred, (int)err };
                awaitable->handle.resume();
            }
        }
    }

    CloseHandle(g_iocp);
    WSACleanup();
    println("Main: Exited cleanly.");
    return 0;
}
