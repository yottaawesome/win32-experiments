module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinUser.h>

export module win32;
import std;

export namespace Win32
{
	using
		::DWORD,
		::HMODULE,
		::HRSRC,
		::LPWSTR,
		::HGLOBAL,
		::LoadLibraryW,
		::GetProcAddress,
		::FindResourceW,
		::LoadResource,
		::SizeofResource,
		::LockResource
		;

	template<auto VValue>
	struct RuntimeValue
	{
		using TValue = std::invoke_result_t<decltype(VValue)>;

		constexpr operator TValue() const noexcept
		{
			return std::invoke(VValue);
		}
	};
	
	namespace ResourceTypes
	{
		constexpr RuntimeValue<[] { return RT_RCDATA; }> RcData;
	}

	constexpr auto MakeIntResource(int data) noexcept
	{
		return MAKEINTRESOURCE(data);
	}
}