module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module win32;

export namespace Win32
{
	using NTSTATUS = ::LONG;
	using 
		::DWORD, 
		::PULONG, 
		::PVOID, 
		::ULONG,
		::HANDLE,
		::HMODULE,
		::HKEY,
		::HKEY__,
		::LSTATUS,
		::GetProcAddress,
		::GetModuleHandleW,
		::GetLastError,
		::RegOpenKeyExW,
		::RegCloseKey
		;

	enum NTStatus : NTSTATUS
	{
		Success = 0x0,
		BufferTooSmall = ((NTSTATUS)0xC0000023L)
	};
	
	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ne-wdm-_key_information_class
	enum KeyInformationClass 
	{
		KeyBasicInformation,
		KeyNodeInformation,
		KeyFullInformation,
		KeyNameInformation,
		KeyCachedInformation,
		KeyFlagsInformation,
		KeyVirtualizationInformation,
		KeyHandleTagsInformation,
		KeyTrustInformation,
		KeyLayerInformation,
		MaxKeyInfoClass
	};

	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-zwquerykey?redirectedfrom=MSDN
	using NtQueryKeyType = DWORD(__stdcall*)(
		HANDLE  KeyHandle,
		int KeyInformationClass,
		PVOID  KeyInformation,
		ULONG  Length,
		PULONG  ResultLength
	);

	template<auto VValue>
	struct RegistryKey
	{
		constexpr operator HKEY() const noexcept
		{
			return VValue;
		}
	};
	constexpr RegistryKey<HKEY_LOCAL_MACHINE> HKLM;

	constexpr auto KeyRead = KEY_READ;
}