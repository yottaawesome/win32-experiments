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
		::PrjStartVirtualizing
		;

	constexpr auto UserDefault = LANG_USER_DEFAULT;
	constexpr auto GenericRead = GENERIC_READ;
	constexpr auto ShareRead = FILE_SHARE_READ;
	constexpr auto OpenExisting = OPEN_EXISTING;
	constexpr auto GenericWrite = GENERIC_WRITE;
	constexpr auto CreateNew = CREATE_NEW;
	constexpr auto FileAttributeHidden = FILE_ATTRIBUTE_HIDDEN;

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