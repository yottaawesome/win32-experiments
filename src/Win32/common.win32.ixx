module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Objbase.h>
#include <projectedfslib.h>
#include <guiddef.h>

export module common.win32;

export namespace Win32
{
	using
		::HRESULT,
		::GUID,
		::PCWSTR,
		::DWORD,
		::HANDLE,
		::PRJ_CALLBACKS,
		::LPSTR,
		::PRJ_NAMESPACE_VIRTUALIZATION_CONTEXT,
		::HMODULE,
		::BOOL,
		::HLOCAL,
		::LONG,
		::LSTATUS,
		::BYTE,
		::LPCVOID,
		::PVOID,
		::LoadLibraryA,
		::LoadLibraryW,
		::FreeLibrary,
		::LocalFree,
		::FormatMessageA,
		::GetLastError,
		::CreateFileW,
		::ReadFile,
		::GetFileSize,
		::CloseHandle,
		::CreateDirectoryW,
		::CoCreateGuid,
		::WriteFile,
		::IsEqualGUID,
		::PrjMarkDirectoryAsPlaceholder,
		::PrjStartVirtualizing,
		::CreateFileW
		;

	constexpr auto UserDefault = LANG_USER_DEFAULT;
	constexpr auto ShareRead = FILE_SHARE_READ;
	constexpr auto OpenExisting = OPEN_EXISTING;
	constexpr auto CreateNew = CREATE_NEW;

	namespace Registry
	{
		using
			::HKEY,
			::RegGetValueW,
			::RegCloseKey,
			::RegOpenKeyExW,
			::RegDeleteKeyW,
			::RegDeleteTreeW,
			::RegCreateKeyExW,
			::RegCloseKey,
			::RegSetValueExW,
			::RegSetValueW,
			::RegSetKeyValueW,
			::RegCreateKeyExW
			;

		namespace ValueTypes
		{
			enum
			{
				String = RRF_RT_REG_SZ,
				DWORD = RRF_RT_REG_DWORD,
				QWORD = RRF_RT_REG_QWORD
			};
		}

		namespace Permissions
		{
			enum
			{
				Read = KEY_READ,
				All = KEY_ALL_ACCESS
			};
		}

		namespace Keys
		{
			template<HKEY T>
			struct Parent { operator HKEY() const noexcept { return T; } };

			const constinit HKEY s = HKEY_LOCAL_MACHINE;

			constexpr Parent<HKEY_LOCAL_MACHINE> HKLM;
			constexpr Parent<HKEY_CURRENT_USER> HKCU;
		}

		namespace Options
		{
			enum
			{
				NonVolatile = REG_OPTION_NON_VOLATILE
			};
		}

		namespace Access
		{
			enum
			{
				AllAccess = KEY_ALL_ACCESS
			};
		}

		namespace Disposition
		{
			enum
			{
				CreatedNewKey = REG_CREATED_NEW_KEY
			};
		}
	}

	namespace Permission
	{
		enum
		{
			GenericRead = GENERIC_READ,
			GenericWrite = GENERIC_WRITE
		};
	}

	namespace FileAttribute
	{
		enum
		{
			Hidden = FILE_ATTRIBUTE_HIDDEN
		};
	}

	namespace FormatMessageFlags
	{
		constexpr auto AllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER;
		constexpr auto FromSystem = FORMAT_MESSAGE_FROM_SYSTEM;
		constexpr auto IgnoreInserts = FORMAT_MESSAGE_IGNORE_INSERTS;
		constexpr auto FromHModule = FORMAT_MESSAGE_FROM_HMODULE;
	}
	
	constexpr ::HRESULT OK = S_OK;

	constexpr GUID NullGuid = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
	namespace Error
	{
		constexpr auto Success = ERROR_SUCCESS;
		constexpr auto AlreadyExists = ERROR_ALREADY_EXISTS;
	}

	const auto InvalidHandleValue = INVALID_HANDLE_VALUE;

	bool HrFailed(::HRESULT hr)
	{
		return FAILED(hr);
	}

	bool HrSucceeded(::HRESULT hr)
	{
		return SUCCEEDED(hr);
	}
}