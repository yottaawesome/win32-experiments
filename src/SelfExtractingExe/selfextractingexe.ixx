module;

#include "../DLLToExtract/exports.hpp"
#include "resource.h"

export module selfextractingexe;
import std;
import win32;

namespace SelfExtractingExe
{
	using FnToImport = decltype(&GetSecretOfTheUniverse);
	using FnToImport2 = decltype(&GetTheOtherSecretOfTheUniverse);

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

		constexpr operator const TChar* () const noexcept
		{
			return Buffer;
		}

		constexpr TView View() const noexcept
		{
			return { Buffer };
		}

		constexpr TString String() const noexcept
		{
			return { Buffer };
		}

		constexpr TChar* Data() noexcept
		{
			return Buffer;
		}

		constexpr const TChar* Data() const noexcept
		{
			return Buffer;
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

	constexpr FixedStringA ExtractedName = "extracted_library.dll";

	struct FmtString
	{
		template<typename...TArgs>
		constexpr FmtString(std::format_string<TArgs...> fmt, TArgs&&...args)
			: Message(std::format(fmt, std::forward<TArgs>(args)...))
		{ }

		std::string Message;

		operator const std::string&() const noexcept
		{
			return Message;
		}
	};

	template<int VResource>
	struct BinaryResource
	{
		BinaryResource()
		{
			Init();
		}

		public:
		const std::byte* Data() const noexcept
		{
			return static_cast<const std::byte*>(m_data);
		}

		Win32::DWORD Size() const noexcept
		{
			return m_size;
		}

		const std::byte* cbegin() const noexcept
		{
			return Data();
		}

		const std::byte* cend() const noexcept
		{
			return Data() + Size();
		}

		const std::byte* begin() const noexcept
		{
			return cbegin();
		}

		const std::byte* end() const noexcept
		{
			return cend();
		}

		void ExtractTo(std::filesystem::path path) const
		{
			std::ofstream outDllFile(path.c_str(), std::ios::out | std::ios::binary | std::ios::app);
			outDllFile.write(reinterpret_cast<const char*>(Data()), Size());
			outDllFile.close();

			if (not std::filesystem::exists(ExtractedName.Data()))
				throw std::runtime_error("Extraction failed.");
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

	template<FixedStringA VLibraryName>
	class DLL
	{
		public:
		~DLL() { Win32::FreeLibrary(m_library); }
		DLL() { Init(); }
		
		DLL(const DLL&) = delete;
		DLL& operator=(const DLL&) = delete;

		public:
		Win32::FARPROC GetAddress(std::string_view name) const
		{
			Win32::FARPROC proc = Win32::GetProcAddress(m_library, name.data());
			if (not proc)
				throw std::runtime_error("GetProcAddress() failed.");
			return proc;
		}

		private:
		void Init()
		{
			if (not std::filesystem::exists(VLibraryName.Data()))
				throw std::runtime_error("DLL does not exist");

			m_library = Win32::LoadLibraryA(VLibraryName.Data());
			if (not m_library)
				throw std::runtime_error(FmtString{ "Failed loading DLL {}", VLibraryName.Data() });
		}

		private:
		Win32::HMODULE m_library = nullptr;
	};
}

export namespace SelfExtractingExe
{
	void ExtractDLL()
	{
		if (std::filesystem::exists(ExtractedName.Data()))
			std::filesystem::remove(ExtractedName.Data());

		BinaryResource<IDR_TEST_DLL> resource;
		resource.ExtractTo(ExtractedName.String());
		
		std::println("Successfully extracted DLL.");
	}

	void LoadDLL()
	{
		DLL<ExtractedName> dll;

		FnToImport fn = reinterpret_cast<FnToImport>(dll.GetAddress("GetSecretOfTheUniverse"));
		if (not fn)
			throw std::runtime_error("Failed to load GetSecretOfTheUniverse()");

		FnToImport2 otherProc = reinterpret_cast<FnToImport2>(dll.GetAddress("GetTheOtherSecretOfTheUniverse"));
		if (not otherProc)
			throw std::runtime_error("Failed to load GetTheOtherSecretOfTheUniverse()");

		std::println("Invoking fn: {}", fn());
		std::println("Invoking otherProc: {}", otherProc());
	}
}
