#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

// Need to link with Ws2_32.lib
// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsasend
#pragma comment(lib, "ws2_32.lib")

#define DATA_BUFSIZE 4096
#define SEND_COUNT   10

int main()
{
    WSADATA wsd;

    struct addrinfo* result = NULL;
    struct addrinfo hints;
    WSAOVERLAPPED SendOverlapped;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET AcceptSocket = INVALID_SOCKET;

    WSABUF DataBuf;
    DWORD SendBytes;
    DWORD Flags;

    char buffer[DATA_BUFSIZE];

    int err = 0;
    int rc, i;

    // Load Winsock
    rc = WSAStartup(MAKEWORD(2, 2), &wsd);
    if (rc != 0) {
        printf("Unable to load Winsock: %d\n", rc);
        return 1;
    }

    // Make sure the hints struct is zeroed out
    SecureZeroMemory((PVOID)&hints, sizeof(struct addrinfo));

    // Initialize the hints to obtain the 
    // wildcard bind address for IPv4
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    rc = getaddrinfo(NULL, "27015", &hints, &result);
    if (rc != 0) {
        printf("getaddrinfo failed with error: %d\n", rc);
        return 1;
    }

    ListenSocket = socket(result->ai_family,
        result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        return 1;
    }

    rc = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (rc == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        return 1;
    }

    rc = listen(ListenSocket, 1);
    if (rc == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        return 1;
    }
    // Accept an incoming connection request
    AcceptSocket = accept(ListenSocket, NULL, NULL);
    if (AcceptSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        return 1;
    }

    printf("Client Accepted...\n");

    // Make sure the SendOverlapped struct is zeroed out
    SecureZeroMemory((PVOID)&SendOverlapped, sizeof(WSAOVERLAPPED));

    // Create an event handle and setup the overlapped structure.
    SendOverlapped.hEvent = WSACreateEvent();
    if (SendOverlapped.hEvent == NULL) {
        printf("WSACreateEvent failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        closesocket(AcceptSocket);
        return 1;
    }

    DataBuf.len = DATA_BUFSIZE;
    DataBuf.buf = buffer;

    for (i = 0; i < SEND_COUNT; i++) {

        rc = WSASend(AcceptSocket, &DataBuf, 1,
            &SendBytes, 0, &SendOverlapped, NULL);
        if ((rc == SOCKET_ERROR) &&
            (WSA_IO_PENDING != (err = WSAGetLastError()))) {
            printf("WSASend failed with error: %d\n", err);
            break;
        }

        rc = WSAWaitForMultipleEvents(1, &SendOverlapped.hEvent, TRUE, INFINITE,
            TRUE);
        if (rc == WSA_WAIT_FAILED) {
            printf("WSAWaitForMultipleEvents failed with error: %d\n",
                WSAGetLastError());
            break;
        }

        rc = WSAGetOverlappedResult(AcceptSocket, &SendOverlapped, &SendBytes,
            FALSE, &Flags);
        if (rc == FALSE) {
            printf("WSASend failed with error: %d\n", WSAGetLastError());
            break;
        }

        printf("Wrote %d bytes\n", SendBytes);

        WSAResetEvent(SendOverlapped.hEvent);

    }

    WSACloseEvent(SendOverlapped.hEvent);
    closesocket(AcceptSocket);
    closesocket(ListenSocket);
    freeaddrinfo(result);

    WSACleanup();

    return 0;
}