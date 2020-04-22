#include "IcmpTests.hpp"
#include <stdexcept>

WinSockInitialiser::WinSockInitialiser()
{
	WSADATA wsaData;
	DWORD errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (errorCode)
		throw std::runtime_error("WSAStartup() failed");
}

WinSockInitialiser::~WinSockInitialiser()
{
	WSACleanup();
}