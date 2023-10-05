#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <Ws2tcpip.h>
import std;
import std.compat;

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

int __cdecl main(int argc, char** argv)
{

    //-----------------------------------------
    // Declare and initialize variables
    WSADATA wsaData;
    int iResult;

    DWORD dwRetval;

    struct sockaddr_in saGNI;
    WCHAR hostname[NI_MAXHOST];
    WCHAR servInfo[NI_MAXSERV];
    u_short port = 27015;

    // Validate the parameters
    if (argc != 2) 
    {
        std::wcout << std::format("usage: {} IPv4 address\n  to return hostname\n       {} 127.0.0.1\n", argv[0], argv[0]).c_str();
        return 1;
    }
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::wcout << std::format(L"WSAStartup failed: {}\n", iResult);
        return 1;
    }
    //-----------------------------------------
    // Set up sockaddr_in structure which is passed
    // to the getnameinfo function
    saGNI.sin_family = AF_INET;
    saGNI.sin_addr.s_addr = inet_addr(argv[1]);
    saGNI.sin_port = htons(port);

    //-----------------------------------------
    // Call GetNameInfoW
    dwRetval = GetNameInfoW((struct sockaddr*)&saGNI,
        sizeof(struct sockaddr),
        hostname,
        NI_MAXHOST, servInfo, NI_MAXSERV, NI_NUMERICSERV);

    if (dwRetval != 0) {
        std::wcout << std::format(L"GetNameInfoW failed with error # {}\n", WSAGetLastError());
        return 1;
    }
    else {
        std::wcout << std::format(L"GetNameInfoW returned hostname = {}\n", hostname);
        return 0;
    }
}