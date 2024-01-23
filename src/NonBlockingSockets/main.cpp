// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsarecv
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
import std;

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

#pragma warning(disable: 4127)  // Conditional expression is a constant

#define DATA_BUFSIZE 4096

int main(int argc, char** argv)
{
    WSADATA wsd;
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    WSAOVERLAPPED RecvOverlapped;
    SOCKET ConnSocket = INVALID_SOCKET;
    WSABUF DataBuf;
    DWORD RecvBytes, Flags;
    char buffer[DATA_BUFSIZE];

    int err = 0;
    int rc;

    if (argc != 2) {
        std::println("usage: {} server-name", argv[0]);
        return 1;
    }
    // Load Winsock
    rc = WSAStartup(MAKEWORD(2, 2), &wsd);
    if (rc != 0) {
        std::println("Unable to load Winsock: {}", rc);
        return 1;
    }
    // Make sure the hints struct is zeroed out
    SecureZeroMemory((PVOID)&hints, sizeof(struct addrinfo));

    // Initialize the hints to retrieve the server address for IPv4
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    rc = getaddrinfo(argv[1], "27015", &hints, &result);
    if (rc != 0) {
        std::println("getaddrinfo failed with error: {}", rc);
        return 1;
    }

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        ConnSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnSocket == INVALID_SOCKET) {
            std::println("socket failed with error: ", WSAGetLastError());
            freeaddrinfo(result);
            return 1;
        }

        rc = connect(ConnSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (rc == SOCKET_ERROR) {

            if (WSAECONNREFUSED == (err = WSAGetLastError())) {
                closesocket(ConnSocket);
                ConnSocket = INVALID_SOCKET;
                continue;
            }
            std::println("connect failed with error: {}\n", err);
            freeaddrinfo(result);
            closesocket(ConnSocket);
            return 1;
        }
        break;
    }
    if (ConnSocket == INVALID_SOCKET) {
        std::println("Unable to establish connection with the server!\n");
        freeaddrinfo(result);
        return 1;
    }

    std::println("Client connected...\n");

    // Make sure the RecvOverlapped struct is zeroed out
    SecureZeroMemory((PVOID)&RecvOverlapped, sizeof(WSAOVERLAPPED));

    // Create an event handle and setup an overlapped structure.
    RecvOverlapped.hEvent = WSACreateEvent();
    if (RecvOverlapped.hEvent == NULL) {
        std::println("WSACreateEvent failed: {}", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ConnSocket);
        return 1;
    }

    DataBuf.len = DATA_BUFSIZE;
    DataBuf.buf = buffer;

    // Call WSARecv until the peer closes the connection
    // or until an error occurs
    while (1) {

        Flags = 0;
        rc = WSARecv(ConnSocket, &DataBuf, 1, &RecvBytes, &Flags, &RecvOverlapped, NULL);
        if ((rc == SOCKET_ERROR) && (WSA_IO_PENDING != (err = WSAGetLastError()))) {
            std::println("WSARecv failed with error: {}", err);
            break;
        }

        rc = WSAWaitForMultipleEvents(1, &RecvOverlapped.hEvent, TRUE, INFINITE, TRUE);
        if (rc == WSA_WAIT_FAILED) {
            std::println("WSAWaitForMultipleEvents failed with error: {}", WSAGetLastError());
            break;
        }

        rc = WSAGetOverlappedResult(ConnSocket, &RecvOverlapped, &RecvBytes, FALSE, &Flags);
        if (rc == FALSE) {
            std::println("WSARecv operation failed with error: {}", WSAGetLastError());
            break;
        }

        std::println("Read {} bytes\n", RecvBytes);

        WSAResetEvent(RecvOverlapped.hEvent);

        // If 0 bytes are received, the connection was closed
        if (RecvBytes == 0)
            break;
    }

    WSACloseEvent(RecvOverlapped.hEvent);
    closesocket(ConnSocket);
    freeaddrinfo(result);

    WSACleanup();

    return 0;
}