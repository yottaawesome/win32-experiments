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

		public:
		const Win32::BYTE* Data() const noexcept
		{
			return static_cast<const BYTE*>(m_data);
		}

		Win32::DWORD Size() const noexcept
		{
			return m_size;
		}

		const Win32::BYTE* cbegin() const noexcept
		{
			return Data();
		}

		const Win32::BYTE* cend() const noexcept
		{
			return Data() + Size();
		}

		const Win32::BYTE* begin() const noexcept
		{
			return cbegin();
		}

		const Win32::BYTE* end() const noexcept
		{
			return cend();
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

			m_data = resourceData;
			m_size = size;
		}

		private:
			Win32::DWORD m_size = 0;
			const void* m_data = nullptr;
	};

	constexpr std::string_view ExtractedName = "extracted_library.dll";

	void ExtractDLL()
	{
		if (std::filesystem::exists(ExtractedName))
			std::filesystem::remove(ExtractedName);

		BinaryResource<IDR_TEST_DLL> resource;
		std::ofstream outDllFile(std::string{ ExtractedName }, std::ios::out | std::ios::binary | std::ios::app);
		outDllFile.write(reinterpret_cast<const char*>(resource.Data()), resource.Size());
		outDllFile.close();

		if (not std::filesystem::exists(ExtractedName))
			throw std::runtime_error("Extraction failed.");
		std::println("Successfully extracted DLL.");
	}

	void LoadDLL()
	{
		Win32::HMODULE hMod = Win32::LoadLibraryA(ExtractedName.data());
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