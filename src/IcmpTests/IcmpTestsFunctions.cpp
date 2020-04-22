#include "IcmpTests.hpp"
#include <stdexcept>
#include <iostream>
#include <vector>

IpAddresses::IpAddresses() {}
IpAddresses::IpAddresses(const std::wstring& ipV4, const std::wstring& ipV6)
:   IpV4(ipV4),
    IpV6(ipV6)
{}

IcmpEchoResult::IcmpEchoResult()
:   RoundTripTime(0),
    DataSize(0),
    Status(0)
{}
IcmpEchoResult::IcmpEchoResult(const ULONG roundTripTime, const USHORT dataSize, const ULONG status)
:   RoundTripTime(roundTripTime),
    DataSize(dataSize),
    Status(status)
{}

IpAddresses HostNameToAddresses(const std::wstring& serverName, const std::wstring& portString)
{
    ADDRINFOW hints{ 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    ADDRINFOW* result = nullptr;
    DWORD dwRetval = GetAddrInfoW(serverName.c_str(), 0, &hints, &result);
    if (dwRetval != 0)
        throw std::runtime_error("GetAddrInfoW failed with error");

    ADDRINFOW* ptr = nullptr;
    IpAddresses ipaddr;
    INT iRetval;
    wchar_t ipstringbuffer[46];
    DWORD ipbufferlength = 46;
    LPSOCKADDR sockaddr_ip;
    for (ptr = result; ptr != nullptr; ptr = ptr->ai_next)
    {
        switch (ptr->ai_family) 
        {
            case AF_UNSPEC:
            {
                std::wcout << L"Unspecified\n" << std::endl;
                break;
            }
            case AF_INET:
            {
                wprintf(L"AF_INET (IPv4)\n");
                sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
                // The buffer length is changed by each call to WSAAddresstoString
                // So we need to set it for each iteration through the loop for safety
                ipbufferlength = 46;
                iRetval = WSAAddressToString(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL,
                    ipstringbuffer, &ipbufferlength);

                std::wstring addr(ipstringbuffer);
                if (iRetval)
                    std::wcout << L"WSAAddressToString failed with " << WSAGetLastError() << std::endl;
                else
                    std::wcout << L"\t" << L"IPv4 address " << addr << std::endl;
                ipaddr.IpV4 = addr;
                break;
            }
            case AF_INET6:
            {
                wprintf(L"AF_INET6 (IPv6)\n");
                // the InetNtop function is available on Windows Vista and later
                // sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
                // printf("\tIPv6 address %s\n",
                //    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );

                // We use WSAAddressToString since it is supported on Windows XP and later
                sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
                // The buffer length is changed by each call to WSAAddresstoString
                // So we need to set it for each iteration through the loop for safety
                ipbufferlength = 46;
                iRetval = WSAAddressToString(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL,
                    ipstringbuffer, &ipbufferlength);
                std::wstring addr(ipstringbuffer);
                if (iRetval)
                    std::wcout << L"WSAAddressToString failed with " << WSAGetLastError() << std::endl;
                else
                    std::wcout << L"\t" << L"IPv6 address " << addr << std::endl;
                ipaddr.IpV6 = addr;
                break;
            }
            default:
                wprintf(L"Other %ld\n", ptr->ai_family);
                break;
        }
    }

    FreeAddrInfoW(result);

    return ipaddr;
}

IcmpEchoResult DoIpV4Ping(const std::wstring& ipAddressToPing)
{
    char SendData[] = "ICMP SEND DATA";
    DWORD replyBufferLength = sizeof(ICMP_ECHO_REPLY) + strlen(SendData) + 1;
    std::vector<char> replyBuffer;
    replyBuffer.resize(replyBufferLength);
    PICMP_ECHO_REPLY pIcmpEchoReply = (PICMP_ECHO_REPLY)&replyBuffer[0];

    IN_ADDR ip4;
    // https://docs.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-inetptonw
    // https://docs.microsoft.com/en-us/windows/win32/api/ipexport/ns-ipexport-icmp_echo_reply
    int inetReturn = InetPton(AF_INET, ipAddressToPing.c_str(), &ip4);
    if (inetReturn == 0)
    {
        throw std::exception("Not a valid IPv4 or IPv6 string");
    }
    else if (inetReturn == -1)
    {
        throw std::exception("InetPton failed with an unknown error", WSAGetLastError());
    }

    IcmpEchoResult result;
    HANDLE hIcmp = IcmpCreateFile();
    DWORD dwRetVal = IcmpSendEcho(
        hIcmp,
        ip4.S_un.S_addr,
        SendData,
        strlen(SendData),
        NULL,
        &replyBuffer[0],
        replyBufferLength,
        1000);

    if (dwRetVal == 0)
    {
        IcmpCloseHandle(hIcmp);
        throw std::exception("Call to IcmpSendEcho() failed");
    }

    result.DataSize = pIcmpEchoReply->DataSize;
    result.Status = pIcmpEchoReply->Status;
    result.RoundTripTime = pIcmpEchoReply->RoundTripTime;

    std::wcout << L"\tRTT: " << result.RoundTripTime << std::endl;
    std::wcout << L"\tStatus: " << result.Status << std::endl;

    IcmpCloseHandle(hIcmp);

    return result;
}

int Old()
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