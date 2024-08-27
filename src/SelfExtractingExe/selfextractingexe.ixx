module;

#include "../DLLToExtract/exports.hpp"
#include "resource.h"

export module selfextractingexe;
import std;
import win32;

using FnToImport = decltype(&GetSecretOfTheUniverse);

export namespace SelfExtractingExe
{
	constexpr std::wstring_view DLLName = L"DLLToExtract";

	template<size_t N, typename TChar, typename TView, typename TString>
	struct FixedString
	{
		TChar Buffer[N]{};

		constexpr FixedString(const TChar(&data)[N])
		{
			std::copy_n(data, N, Buffer);
		}

		constexpr operator TView() const noexcept
		{
			return { Buffer };
		}

		constexpr operator TString() const noexcept
		{
			return { Buffer };
		}

		constexpr TView View() const noexcept
		{
			return { Buffer };
		}

		constexpr TString String() const noexcept
		{
			return { Buffer };
		}
	};
	template<size_t N>
	FixedString(const char(&)[N]) -> FixedString<N, char, std::string_view, std::string>;
	template<size_t N>
	FixedString(const wchar_t(&)[N]) -> FixedString<N, wchar_t, std::wstring_view, std::wstring>;

	template<size_t N>
	using FixedStringW = FixedString<N, wchar_t, std::wstring_view, std::wstring>;
	template<size_t N>
	using FixedStringA = FixedString<N, char, std::string_view, std::string>;

	template<int VResource>
	struct BinaryResource
	{
		BinaryResource()
		{
			Init();
		}

		private:
		void Init()
		{
			Win32::HRSRC rsc = Win32::FindResourceW(
				nullptr, 
				Win32::MakeIntResource(VResource), 
				static_cast<Win32::LPWSTR>(Win32::ResourceTypes::RcData)
			);
			if (not rsc)
				throw std::runtime_error("FindResourceW() failed");

			Win32::HGLOBAL hgRsc = Win32::LoadResource(nullptr, rsc);
			if (not hgRsc)
				throw std::runtime_error("LoadResource() failed");

			const void* resourceData = Win32::LockResource(hgRsc);
			if (not resourceData)
				throw std::runtime_error("LockResource() failed");

			Win32::DWORD size = Win32::SizeofResource(nullptr, rsc);
			if (size == 0)
				throw std::runtime_error("SizeofResource() failed");
		}
	};

	void ExtractDLL()
	{
		BinaryResource<IDR_TEST_DLL> data;
	}

	void LoadDLL()
	{
		Win32::HMODULE hMod = Win32::LoadLibraryW(DLLName.data());
		if (not hMod)
			throw std::runtime_error("Failed loading DLL");
		FnToImport fn = reinterpret_cast<FnToImport>(Win32::GetProcAddress(hMod, "GetSecretOfTheUniverse"));
		if (not fn)
			throw std::runtime_error("Failed to load GetSecretOfTheUniverse()");

		auto otherProc = Win32::GetProcAddress(hMod, "GetTheOtherSecretOfTheUniverse");
		if (not otherProc)
			throw std::runtime_error("Failed to load GetTheOtherSecretOfTheUniverse()");
	}
}