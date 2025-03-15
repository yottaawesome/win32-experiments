module;

#define WIN32_LEAN_AND_MEAN
#define SECURITY_WIN32

#include <Windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <sspi.h>

export module Win32;

export namespace Win32
{
	using
		::SOCKET,
		::BYTE,
		::PCHAR,
		::WSADATA,
		::CredHandle,
		::_SecHandle,
		::SECURITY_STATUS,
		::DWORD,
		::ULONG,
		::SecPkgContext_Sizes,
		::SecPkgContext_NegotiationInfoW,
		::BOOL,
		::SOCKADDR_IN,
		::LPSOCKADDR,
		::hostent,
		::IN_ADDR,
		::GetAddrInfoW,
		::InetPtonW,
		::inet_pton,
		::WSAStartup,
		::socket,
		::connect,
		::closesocket,
		::gethostbyname,
		::inet_addr,
		::htons
		;

	auto MakeWord()
	{
		return MAKEWORD(2, 2);
	}

	constexpr auto PfInet = PF_INET;
	constexpr auto AfInet = AF_INET;
	constexpr auto SockStream = SOCK_STREAM;
	constexpr auto InvalidSocket = INVALID_SOCKET;
	constexpr auto InAddrNone = INADDR_NONE;
}
