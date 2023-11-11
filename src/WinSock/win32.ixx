module;

#define WIN32_LEAN_AND_MEAN // necessary to prevent redefinitions with winsock headers
#include <Windows.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Mstcpip.h>
#include <Ip2string.h>
#include <nspapi.h>
#include <Wsrm.h>
#include <ws2bth.h>
#include <winioctl.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
// Required for RtlIpv4StringToAddressW, otherwise has to be manually GetProcAddress()ed.
#pragma comment(lib, "onecoreuap.lib")
#pragma comment(lib, "Mswsock.lib")

export module win32;

export namespace Win32
{
	using 
		::DWORD,
		::HMODULE,
		::WORD;

	using ::GetLastError;
	using ::FormatMessageA;
	using ::GetLastError;
	using ::LocalFree;
	using ::LoadLibrary;
	using ::LoadLibraryW;
	using ::FreeLibrary;

	namespace FormatMessageFlags
	{
		enum
		{
			AllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER,
			MessageFromSystem = FORMAT_MESSAGE_FROM_SYSTEM,
			IgnoreInserts = FORMAT_MESSAGE_IGNORE_INSERTS,
			FromHModule = FORMAT_MESSAGE_FROM_HMODULE
		};
	}

	constexpr auto MakeWord(int a, int b) noexcept
	{
		return MAKEWORD(a, b);
	}

	namespace WinSock
	{
		using 
			::ADDRINFOW,
			::SOCKET,
			::WSADATA,
			::SOCKADDR;

		constexpr auto InvalidSocket = INVALID_SOCKET;

		enum class AddressFamily
		{
			Unspec = AF_UNSPEC,
			IRDA = AF_IRDA,
			Blutooth = AF_BTH,
			Inet = AF_INET,
			Inet6 = AF_INET6,
		};

		enum class SocketTypes
		{
			DGram = SOCK_DGRAM,
			Raw = SOCK_RAW,
			Rdm = SOCK_RDM,
			SeqPackt = SOCK_SEQPACKET,
			Stream = SOCK_STREAM,
		};

		enum class Protocol
		{
			ICMP = IPPROTO_ICMP,
			IGMP = IPPROTO_IGMP,
			Blutooth = BTHPROTO_RFCOMM,
			TCP = IPPROTO_TCP
		};

		using 
			::GetAddrInfoA,
			::GetAddrInfoW,
			::FreeAddrInfoA,
			::FreeAddrInfoW,
			::socket,
			::RtlIpv4StringToAddressA,
			::RtlIpv4StringToAddressW,
			::closesocket,
			::WSAGetLastError,
			::WSAStartup,
			::WSACleanup,
			::WSARecv,
			::WSASend,
			::recv, 
			::send,
			::WSADuplicateSocket,
			::bind,
			::connect,
			::bind,
			::WSAIoctl,
			::gethostbyaddr,
			::gethostbyname,
			::gethostname,
			::GetHostNameW,
			::GetNameByType,
			::getnameinfo,
			::getipv4sourcefilter,
			::GetNameInfoW,
			::getpeername,
			::getprotobyname,
			::getprotobynumber,
			::getservbyname,
			::setsockopt,
			::getsockopt;
	}
}
