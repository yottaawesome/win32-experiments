module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Objbase.h>
#include <projectedfslib.h>
#include <guiddef.h>

export module win32;

export namespace Win32
{
	using
		::HRESULT,
		::GUID,
		::PCWSTR,
		::DWORD,
		::HANDLE,
		::PRJ_CALLBACKS,
		::PRJ_NAMESPACE_VIRTUALIZATION_CONTEXT,
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

	constexpr auto GenericRead = GENERIC_READ;
	constexpr auto ShareRead = FILE_SHARE_READ;
	constexpr auto OpenExisting = OPEN_EXISTING;
	constexpr auto GenericWrite = GENERIC_WRITE;
	constexpr auto CreateNew = CREATE_NEW;
	constexpr auto FileAttributeHidden = FILE_ATTRIBUTE_HIDDEN;

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