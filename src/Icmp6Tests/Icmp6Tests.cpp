#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <ws2tcpip.h>

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "ws2_32.lib")

struct IpAddresses final
{
    std::wstring IpV4;
    std::wstring IpV6;
};

IpAddresses HostNameToAddresses(const std::wstring& serverName, const std::wstring& portString);

// See also
// https://github.com/crossbear/Crossbear/blob/864b3b07d6cdbb5f16fab6707963de8af31966d0/ffplugin/libraries/crossbear.cpp
// https://github.com/xbmc/xbmc/blob/770c691bfeea88d561b39d2e38348ac434db3adc/xbmc/platform/win32/network/NetworkWin32.cpp
// https://github.com/atlan-antillia/sol9/blob/5e9f64266db899cdde57d65198563b8597fab0c2/sol9_class_library/usr/include/sol/icmp/IcmpV6File.h
// https://docs.microsoft.com/en-us/windows/win32/api/in6addr/ns-in6addr-in6_addr
// https://docs.microsoft.com/en-us/windows/win32/api/icmpapi/nf-icmpapi-icmp6createfile
// https://docs.microsoft.com/en-us/windows/win32/api/icmpapi/nf-icmpapi-icmp6sendecho2


int main()
{
    WSADATA wsaData;
    DWORD errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (errorCode)
        throw std::runtime_error("WSAStartup() failed");

    HANDLE hIcmpFile = Icmp6CreateFile();

	SOCKADDR_IN6			src{ 0 };

    std::wstring hostname = L"www.google.com";
    IpAddresses ipaddressToPing = HostNameToAddresses(hostname, L"80");
    IN6_ADDR ip6dest;
    int inetReturn2 = InetPton(AF_INET6, ipaddressToPing.IpV6.c_str(), &ip6dest);
    sockaddr_in6 dest{ 0 };
    dest.sin6_addr = ip6dest;
    dest.sin6_family = AF_INET6;
    dest.sin6_port = 0;

    char sendData[] = "ICMP SEND DATA";
    DWORD replyBufferLength = sizeof(ICMPV6_ECHO_REPLY) + strlen(sendData) + 1;
    std::vector<char> replyBuffer;
    replyBuffer.resize(replyBufferLength);

    DWORD result = Icmp6SendEcho2(
        hIcmpFile,
        nullptr,
        nullptr,
        nullptr,
        &src,
        &dest,
        sendData,
        strlen(sendData),
        nullptr,
        &replyBuffer[0],
        replyBuffer.size(),
        1000
    );

    std::wcout << result << std::endl;
    std::wcout << GetLastError() << std::endl;

    return 0;
}

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
