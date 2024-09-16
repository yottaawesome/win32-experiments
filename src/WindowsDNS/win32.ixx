module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ws2tcpip.h>
#include <WinDNS.h>

#pragma comment(lib, "Dnsapi.lib")
#pragma comment(lib, "ws2_32.lib")

export module win32;
import std;

export namespace Win32
{
	using
		::GetAddrInfoExW,	// https://learn.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfoexw
		::DnsQuery_W,		// https://learn.microsoft.com/en-us/windows/win32/api/windns/nf-windns-dnsquery_w
		::DnsQueryEx,		// https://learn.microsoft.com/en-us/windows/win32/api/windns/nf-windns-dnsqueryex
		::FreeAddrInfoW,
		::DnsFree,
		::FormatMessageA,
		::FormatMessageW,
		::LocalFree,
		::CreateEventW,
		::CloseHandle,
		::SetEvent,
		::WaitForSingleObject,
		::GetLastError,
		::InetNtopW,		 // https://learn.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-inetntopw
		::WSAAddressToStringW,
		::WideCharToMultiByte,
		::WSAStartup,
		::GetCurrentThread,
		::SleepEx,
		::QueueUserAPC,
		::OpenThread,
		::GetCurrentThreadId,
		::ULONG_PTR,
		::DNS_FREE_TYPE,
		::DWORD,
		::HANDLE,
		::DNS_QUERY_CANCEL,
		::DNS_QUERY_REQUEST, // https://learn.microsoft.com/en-us/windows/win32/api/windns/ns-windns-dns_query_request
		::DNS_QUERY_RESULT,
		::DNS_STATUS,
		::DNS_RECORD,
		::ADDRINFOEXW,
		::timeval,
		::OVERLAPPED,
		::WSADATA
		;

	constexpr auto ThreadSetContext = THREAD_SET_CONTEXT;

	namespace ErrorCodes
	{
		enum
		{
			Success = ERROR_SUCCESS,
			DnsRequestPending = DNS_REQUEST_PENDING,
			WsaIoPending = WSA_IO_PENDING
		};
	}

	constexpr auto CpUtf8 = CP_UTF8;
	constexpr auto WcNoBestFitChars = WC_NO_BEST_FIT_CHARS;

	constexpr auto SocketError = SOCKET_ERROR;

	enum FormatMessageOptions
	{
		AllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER,
		FromSystem = FORMAT_MESSAGE_FROM_SYSTEM,
		IgnoreInserts = FORMAT_MESSAGE_IGNORE_INSERTS
	};

	enum ServiceNamespace
	{
		DNS = NS_DNS
	};

	enum IPAddressFamily
	{
		v4 = AF_INET,
		v6 = AF_INET6
	};

	constexpr auto DnsQueryRequestVersion1 = DNS_QUERY_REQUEST_VERSION1;

	// https://learn.microsoft.com/en-us/windows/win32/dns/dns-constants
	enum DnsRecordType
	{
		A = DNS_TYPE_A,
		AAAA = DNS_TYPE_AAAA
	};

	enum DnsOptions
	{
		DnsQueryStandard = DNS_QUERY_STANDARD,
		DualAddress = DNS_QUERY_DUAL_ADDR
	};

	constexpr auto InfiniteWait = INFINITE;

	constexpr auto MakeWord(int a, int b)
	{
		return MAKEWORD(a, b);
	}
}

export namespace Util
{
	struct LocalFreeDeleter { void operator()(HLOCAL ptr) const noexcept { LocalFree(ptr); } };
	using LocalUniquePtr = std::unique_ptr<std::remove_pointer_t<HLOCAL>, LocalFreeDeleter>;

	std::string ErrorCodeToString(DWORD errorCode) noexcept
	{
		void* messageBuffer = nullptr;
		DWORD result = Win32::FormatMessageA(
			Win32::FormatMessageOptions::AllocateBuffer | Win32::FormatMessageOptions::FromSystem | Win32::FormatMessageOptions::IgnoreInserts,
			nullptr, 
			errorCode, 
			0,
			reinterpret_cast<char*>(&messageBuffer),
			0,
			nullptr
		);
		if (not result)
			return std::format("Failed formatting message from error code {}.", errorCode);

		LocalUniquePtr ptr(messageBuffer);
		std::string returnValue(reinterpret_cast<char*>(messageBuffer), result);

		return returnValue;
	}

	struct Win32Error : public std::runtime_error
	{
		Win32Error(std::string_view msg, const std::source_location loc = std::source_location::current()) 
			: std::runtime_error(Format(msg, loc)) {}
		Win32Error(Win32::DWORD errorCode, std::string_view msg, const std::source_location loc = std::source_location::current())
			: m_code(errorCode), std::runtime_error(Format(errorCode, msg, loc)) {}

		static std::string Format(std::string_view msg, const std::source_location loc) noexcept
			{ return std::format("{} -> {}:{}:{}", msg, loc.function_name(), loc.file_name(), loc.line()); }

		static std::string Format(Win32::DWORD errorCode, std::string_view msg, const std::source_location loc) noexcept
			{ return std::format("{} -> {} -> {}:{}:{}", msg, ErrorCodeToString(errorCode), loc.function_name(), loc.file_name(), loc.line()); }

		Win32::DWORD Code() const noexcept { return m_code; }

		private:
			Win32::DWORD m_code = 0;
	};
}