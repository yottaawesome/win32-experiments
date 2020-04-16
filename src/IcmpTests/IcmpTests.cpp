//#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include <iostream>
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <stdio.h>
#include <ws2tcpip.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

//https://docs.microsoft.com/en-us/windows/win32/api/icmpapi/nf-icmpapi-icmpsendecho
// https://www.geekpage.jp/en/programming/iphlpapi/send-icmp.php
int main(int argc, char** argv)
{
    HANDLE hIcmp;
    char SendData[] = "ICMP SEND DATA";
    LPVOID ReplyBuffer;
    DWORD dwRetVal;
    DWORD buflen;
    PICMP_ECHO_REPLY pIcmpEchoReply;

    hIcmp = IcmpCreateFile();

    buflen = sizeof(ICMP_ECHO_REPLY) + strlen(SendData) + 1;
    ReplyBuffer = (VOID*)malloc(buflen);
    if (ReplyBuffer == NULL) {
        return 1;
    }
    memset(ReplyBuffer, 0, buflen);
    pIcmpEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;

    IN_ADDR ip4;
    InetPton(AF_INET, L"127.0.0.1", &ip4);
    dwRetVal = IcmpSendEcho(hIcmp,
        //inet_addr("127.0.0.1"),
        ip4.S_un.S_addr,
        SendData, 
        strlen(SendData),
        NULL,
        ReplyBuffer,
        buflen,
        1000);
    if (dwRetVal != 0) {
        printf("Received %ld messages.\n", dwRetVal);
        printf("\n");
        printf("RTT: %d\n", pIcmpEchoReply->RoundTripTime);
        printf("Data Size: %d\n", pIcmpEchoReply->DataSize);
        printf("Message: %s\n", pIcmpEchoReply->Data);
    }
    else {
        printf("Call to IcmpSendEcho() failed.\n");
        printf("Error: %ld\n", GetLastError());
    }

    IcmpCloseHandle(hIcmp);

    return 0;
}
