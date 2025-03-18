export module common;
import std;
import win32;

export namespace Utl
{
	auto TranslateErrorCode(Win32::DWORD errorCode) -> std::string
	{
		constexpr Win32::DWORD flags =
			Win32::FormatMessageFlags::AllocateBuffer
			| Win32::FormatMessageFlags::FromSystem
			| Win32::FormatMessageFlags::IgnoreInserts;

		void* messageBuffer = nullptr;
		Win32::FormatMessageA(
			flags,
			nullptr,
			errorCode,
			0,
			reinterpret_cast<char*>(&messageBuffer),
			0,
			nullptr
		);
		if (not messageBuffer)
			return std::format("FormatMessageA() failed on code {} with error {}", errorCode, Win32::GetLastError());

		std::string msg(static_cast<char*>(messageBuffer));
		// This should never happen
		// See also https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-raisefailfastexception
		if (Win32::LocalFree(messageBuffer))
			std::abort();

		std::erase_if(msg, [](const char x) { return x == '\n' || x == '\r'; });
		return msg;
	}
}

export namespace Error
{
	struct Win32Error : std::runtime_error
	{
		Win32Error(
			Win32::DWORD code,
			std::string_view msg,
			const std::source_location& loc = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current()
		) : runtime_error(Utl::TranslateErrorCode(code))
		{
		}
	};
}

export namespace Utl
{
	std::string ConvertString(std::wstring_view wstr)
	{
		if (wstr.empty())
			return {};

		// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte
		// Returns the size in bytes, this differs from MultiByteToWideChar, which returns the size in characters
		const int sizeInBytes = Win32::WideCharToMultiByte(
			Win32::CpUtf8,										// CodePage
			Win32::WcNoBestFitChars,							// dwFlags 
			&wstr[0],										// lpWideCharStr
			static_cast<int>(wstr.size()),					// cchWideChar 
			nullptr,										// lpMultiByteStr
			0,												// cbMultiByte
			nullptr,										// lpDefaultChar
			nullptr											// lpUsedDefaultChar
		);
		if (sizeInBytes == 0)
			throw Error::Win32Error(Win32::GetLastError(), "WideCharToMultiByte() [1] failed");

		std::string strTo(sizeInBytes / sizeof(char), '\0');
		const int status = WideCharToMultiByte(
			Win32::CpUtf8,										// CodePage
			Win32::WcNoBestFitChars,							// dwFlags 
			&wstr[0],										// lpWideCharStr
			static_cast<int>(wstr.size()),					// cchWideChar 
			&strTo[0],										// lpMultiByteStr
			static_cast<int>(strTo.size() * sizeof(char)),	// cbMultiByte
			nullptr,										// lpDefaultChar
			nullptr											// lpUsedDefaultChar
		);
		if (status == 0)
			throw Error::Win32Error(Win32::GetLastError(), "WideCharToMultiByte() [2] failed");

		return strTo;
	}

	std::wstring ConvertString(std::string_view str)
	{
		if (str.empty())
			return {};

		// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
		// Returns the size in characters, this differs from WideCharToMultiByte, which returns the size in bytes
		int sizeInCharacters = Win32::MultiByteToWideChar(
			Win32::CpUtf8,									// CodePage
			0,											// dwFlags
			&str[0],									// lpMultiByteStr
			static_cast<int>(str.size() * sizeof(char)),// cbMultiByte
			nullptr,									// lpWideCharStr
			0											// cchWideChar
		);
		if (sizeInCharacters == 0)
			throw Error::Win32Error(Win32::GetLastError(), "MultiByteToWideChar() [1] failed");

		std::wstring wstrTo(sizeInCharacters, '\0');
		int status = Win32::MultiByteToWideChar(
			Win32::CpUtf8,									// CodePage
			0,											// dwFlags
			&str[0],									// lpMultiByteStr
			static_cast<int>(str.size() * sizeof(char)),	// cbMultiByte
			&wstrTo[0],									// lpWideCharStr
			static_cast<int>(wstrTo.size())				// cchWideChar
		);
		if (status == 0)
			throw Error::Win32Error(Win32::GetLastError(), "MultiByteToWideChar() [2] failed");

		return wstrTo;
	}
}

export namespace RAII
{
	template<auto VDeleteFn>
	struct Deleter
	{
		void operator()(auto handle) const noexcept
		{
			VDeleteFn(handle);
		}
	};

	template<typename T, auto VDeleter>
	using UniquePtr = std::unique_ptr<T, Deleter<VDeleter>>;
	template<typename T, auto VDeleter>
	using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<T>, Deleter<VDeleter>>;

	using ServiceUniquePtr = IndirectUniquePtr<Win32::SC_HANDLE, Win32::CloseServiceHandle>;
	using HandleUniquePtr = IndirectUniquePtr<Win32::HANDLE, Win32::CloseHandle>;
	using EnvironmentUniquePtr = UniquePtr<void, Win32::CloseHandle>;
}
