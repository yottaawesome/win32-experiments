//#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include <iostream>
#include "IcmpTests.hpp"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

//https://docs.microsoft.com/en-us/windows/win32/api/icmpapi/nf-icmpapi-icmpsendecho
// https://www.geekpage.jp/en/programming/iphlpapi/send-icmp.php
int main(int argc, char** argv)
{
    WinSockInitialiser ws;
    IpAddresses ip = HostNameToAddresses(L"www.google.com", L"80");
    DoIpV4Ping(ip.IpV4);
    return 0;
}
