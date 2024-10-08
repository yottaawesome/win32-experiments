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

	template<typename TError, typename...TArgs>
	struct Check
	{
		constexpr static std::string_view Format = 
R"({} at 
	function: {}
	file: {}
	line: {})";
		Check(
			bool condition,
			std::format_string<TArgs...> fmt,
			TArgs&&...args,
			const std::source_location loc = std::source_location::current()
		)
		{
			if (condition)
				return;
			throw TError(
				std::format(
					Format,
					std::format(fmt, std::forward<TArgs>(args)...),
					loc.function_name(),
					loc.file_name(),
					loc.line()
				)
			);
		}
	};
	template<typename TError, typename...TArgs>
	Check(bool, std::format_string<TArgs...>, TArgs&&...)->Check<TError, TArgs...>;

	export struct RuntimeError : public std::runtime_error
	{
		RuntimeError(std::string s) : std::runtime_error(s) {}

		template<typename...TArgs>
		using Check = Check<RuntimeError, TArgs...>;
	};

	struct Win32Error : public std::runtime_error
	{
		Win32Error(std::string s) : std::runtime_error(s) {}

		template<typename...TArgs>
		struct Check
		{
			constexpr static std::string_view Format = 
R"({} - Win32 error {} - at 
	function: {}
	file: {}
	line: {})";
			Check(
				bool condition,
				std::format_string<TArgs...> fmt,
				TArgs&&...args,
				const std::source_location loc = std::source_location::current()
			)
			{
				if (condition)
					return;
				const auto lastError = Win32::GetLastError();
				throw Win32Error(
					std::format(
						Format,
						std::format(fmt, std::forward<TArgs>(args)...),
						lastError,
						loc.function_name(),
						loc.file_name(),
						loc.line()
					)
				);
			}
		};
		template<typename...TArgs>
		Check(bool, std::format_string<TArgs...>, TArgs&&...) -> Check<TArgs...>;
	};

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

			const bool success = std::filesystem::exists(ExtractedName.Data());
			RuntimeError::Check(success, "Extraction failed for resource {}.", VResource);
		}

		private:
		void Init()
		{
			Win32::HRSRC rsc = Win32::FindResourceW(
				nullptr, 
				Win32::MakeIntResource(VResource), 
				static_cast<Win32::LPWSTR>(Win32::ResourceTypes::RcData)
			);
			RuntimeError::Check(rsc, "FindResourceW() failed");

			Win32::HGLOBAL hgRsc = Win32::LoadResource(nullptr, rsc);
			RuntimeError::Check(hgRsc, "LoadResource() failed");

			const void* resourceData = Win32::LockResource(hgRsc);
			RuntimeError::Check(hgRsc, "LockResource() failed");

			Win32::DWORD size = Win32::SizeofResource(nullptr, rsc);
			RuntimeError::Check(size > 0, "SizeofResource() failed");

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
			RuntimeError::Check(proc, "GetProcAddress() failed/");
			return proc;
		}

		private:
		void Init()
		{
			RuntimeError::Check(std::filesystem::exists(VLibraryName.Data()), "DLL does not exist");
			m_library = Win32::LoadLibraryA(VLibraryName.Data());
			Win32Error::Check(m_library, "Failed loading DLL {}", VLibraryName.Data());
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
		RuntimeError::Check(fn, "Failed to load GetSecretOfTheUniverse()");

		FnToImport2 otherProc = reinterpret_cast<FnToImport2>(dll.GetAddress("GetTheOtherSecretOfTheUniverse"));
		RuntimeError::Check(otherProc, "Failed to load GetTheOtherSecretOfTheUniverse()");

		std::println("Invoking fn: {}", fn());
		std::println("Invoking otherProc: {}", otherProc());
	}
}
