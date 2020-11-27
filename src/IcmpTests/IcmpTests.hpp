#pragma once
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <string>

class WinSockInitialiser final
{
	public:
		WinSockInitialiser();
		~WinSockInitialiser();
};

class IpAddresses final
{
	public:
		IpAddresses();
		IpAddresses(const std::wstring& ipV4, const std::wstring& ipV6);
		std::wstring IpV4;
		std::wstring IpV6;
};

class IcmpEchoResult final
{
	public:
		IcmpEchoResult();
		IcmpEchoResult(const ULONG roundTripTime, const USHORT dataSize, const ULONG status);
		ULONG RoundTripTime;
		USHORT DataSize;
		ULONG Status;
};

IcmpEchoResult DoIpV4Ping(const std::wstring& ipAddressToPing);
IcmpEchoResult DoIpV6Ping(const std::wstring& ipAddressToPing);
IpAddresses HostNameToAddresses(const std::wstring& serverName, const std::wstring& portString);

int Old();