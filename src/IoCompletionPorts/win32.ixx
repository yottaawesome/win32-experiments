module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module win32;
import std;

export namespace Win32
{
	using
		::HANDLE,
		::ULONG_PTR,
		::OVERLAPPED,
		::BYTE,
		::DWORD,
		::HRESULT,
		::LPSTR,
		::PostQueuedCompletionStatus,
		::GetQueuedCompletionStatus,
		::CreateFileW,
		::ReadFile,
		::CloseHandle,
		::CreateIoCompletionPort,
		::GetLastError,
		::CreateFile,
		::FormatMessageA,
		::LocalFree
		;

	constexpr auto FormatMessageAllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER;
	constexpr auto FormatMessageFromSystem = FORMAT_MESSAGE_FROM_SYSTEM;
	constexpr auto FormatMessageIgnoreInserts = FORMAT_MESSAGE_IGNORE_INSERTS;

	struct InvalidHandleValueT
	{
		constexpr operator HANDLE() const noexcept
		{
			return INVALID_HANDLE_VALUE;
		}
		constexpr bool operator==(HANDLE other) const noexcept
		{
			return INVALID_HANDLE_VALUE == other;
		}
	} constexpr InvalidHandleValue;

	HRESULT HresultFromWin32(DWORD error) noexcept
	{
		return HRESULT_FROM_WIN32(error);
	}

	template<auto VValue>
	struct HResult
	{
		constexpr operator HRESULT() const noexcept
		{
			return VValue;
		}
	};
	namespace HResults
	{
		constexpr HResult<S_OK> OK;
	}

	constexpr auto Failed(HRESULT hr) noexcept
	{
		return FAILED(hr);
	}

	constexpr auto Infinite = INFINITE;
	constexpr auto FileReadData = FILE_READ_DATA;
	constexpr auto FileShareRead = FILE_SHARE_READ;
	constexpr auto OpenExisting = OPEN_EXISTING;
	constexpr auto FileFlagOverlapped = FILE_FLAG_OVERLAPPED;	
}

export namespace Win32::Error
{
	std::string TranslateError(Win32::DWORD code) noexcept
	{
		void* buffer = nullptr;
		Win32::DWORD count = Win32::FormatMessageA(
			Win32::FormatMessageAllocateBuffer | Win32::FormatMessageFromSystem | Win32::FormatMessageIgnoreInserts,
			nullptr,
			code,
			0,
			reinterpret_cast<Win32::LPSTR>(&buffer),
			0,
			nullptr
		);
		if (not buffer)
			return std::format("Failed interpreting {}", code);
		std::string message(reinterpret_cast<char*>(buffer));
		Win32::LocalFree(buffer);
		return message;
	}

	struct Win32ErrorCode
	{
		constexpr Win32ErrorCode() noexcept = default;

		constexpr Win32ErrorCode(DWORD value) noexcept
			: m_value(value)
		{ }

		constexpr operator DWORD() const noexcept
		{
			return m_value;
		}

		constexpr bool operator==(DWORD other) const noexcept
		{
			return m_value == other;
		}

		std::string ToString() const noexcept
		{
			return TranslateError(m_value);
		}

		constexpr DWORD Code() const noexcept
		{
			return m_value;
		}

		DWORD m_value = 0;
	};

	namespace Codes
	{
		constexpr Win32ErrorCode IoPending = ERROR_IO_PENDING;
	}

	struct Win32Error : public std::runtime_error
	{
		Win32Error(
			Win32::DWORD code = Win32::GetLastError(),
			std::source_location loc = std::source_location::current()
		) noexcept : std::runtime_error(Format(code, loc))
		{}

		static std::string Format(Win32::DWORD code, const std::source_location& loc) noexcept
		{
			return std::format(
				"Win32 error: {} at {}:{}:{}",
				Win32::Error::TranslateError(code),
				loc.function_name(),
				loc.file_name(),
				loc.line()
			);
		}
	};
}
