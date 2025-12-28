module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <pathcch.h>

export module filelockmutex:win32;

export namespace Win32
{
	using
		::HANDLE,
		::DWORD,
		::STARTUPINFOW,
		::PROCESS_INFORMATION,
		::HRESULT,
		::PathCchRemoveFileSpec,
		::GetModuleFileNameW,
		::WaitForSingleObject,
		::WaitForSingleObjectEx,
		::WaitForMultipleObjects,
		::WaitForMultipleObjectsEx,
		::GetLastError,
		::CreateFileW,
		::CloseHandle,
		::CreateProcessW
		;

	template<auto T>
	struct Win32Constant
	{
		static constexpr auto operator()() noexcept -> decltype(T)
		{
			return T;
		}

		constexpr operator decltype(T)() const noexcept
		{
			return T;
		}
	};

	constexpr Win32Constant<INVALID_HANDLE_VALUE> InvalidHandleValue;
	constexpr DWORD Infinite = INFINITE;
	constexpr HRESULT SOk = S_OK;
	constexpr HRESULT SFalse = S_FALSE;

	namespace ErrorCodes
	{
		enum
		{
			InsufficientBuffer = ERROR_INSUFFICIENT_BUFFER,
		};
	}

	namespace WaitResult
	{
		enum
		{
			Abandoned = WAIT_ABANDONED,
			Object0 = WAIT_OBJECT_0,
			Timeout = WAIT_TIMEOUT,
			Failed = WAIT_FAILED,
		};
	}

	enum class FileShareMode : DWORD
	{
		None = 0,
		Read = FILE_SHARE_READ,
		Write = FILE_SHARE_WRITE,
		Delete = FILE_SHARE_DELETE,
	};

	constexpr auto GenericRead = GENERIC_READ;

	enum class CreateFileDisposition : DWORD
	{
		CreateNew = CREATE_NEW,
		CreateAlways = CREATE_ALWAYS,
		OpenExisting = OPEN_EXISTING,
		OpenAlways = OPEN_ALWAYS,
		TruncateExisting = TRUNCATE_EXISTING,
	};

	namespace FileAttributes
	{
		enum
		{
			Normal = FILE_ATTRIBUTE_NORMAL,
		};
	}

	namespace FileFlags
	{
		enum
		{
			DeleteOnClose = FILE_FLAG_DELETE_ON_CLOSE,
		};
	}
}
