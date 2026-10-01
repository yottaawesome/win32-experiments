export module common;
import std;
import win32;

export namespace Utl
{
	auto TranslateErrorCode(Win32::DWORD errorCode) -> std::string
	{
		constexpr auto flags =
			Win32::FormatMessageFlags::AllocateBuffer
			| Win32::FormatMessageFlags::FromSystem
			| Win32::FormatMessageFlags::IgnoreInserts;

		auto messageBuffer = static_cast<void*>(nullptr);
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

		auto msg = std::string(static_cast<char*>(messageBuffer));
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
		) : runtime_error(std::format("{}: {}", msg, Utl::TranslateErrorCode(code)))
		{ }
	};
}

export namespace Utl
{
	auto ConvertString(std::wstring_view wstr) -> std::string
	{
		if (wstr.empty())
			return {};

		// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte
		// Returns the size in bytes, this differs from MultiByteToWideChar, which returns the size in characters
		auto sizeInBytes = Win32::WideCharToMultiByte(
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
			throw Error::Win32Error{Win32::GetLastError(), "WideCharToMultiByte() [1] failed"};

		auto strTo = std::string(sizeInBytes / sizeof(char), '\0');
		auto status = WideCharToMultiByte(
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
			throw Error::Win32Error{Win32::GetLastError(), "WideCharToMultiByte() [2] failed"};

		return strTo;
	}

	auto ConvertString(std::string_view str) -> std::wstring
	{
		if (str.empty())
			return {};

		// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
		// Returns the size in characters, this differs from WideCharToMultiByte, which returns the size in bytes
		auto sizeInCharacters = Win32::MultiByteToWideChar(
			Win32::CpUtf8,									// CodePage
			0,											// dwFlags
			str.data(),									// lpMultiByteStr
			static_cast<int>(str.size() * sizeof(char)),// cbMultiByte
			nullptr,									// lpWideCharStr
			0											// cchWideChar
		);
		if (sizeInCharacters == 0)
			throw Error::Win32Error{Win32::GetLastError(), "MultiByteToWideChar() [1] failed"};

		auto wstrTo = std::wstring(sizeInCharacters, '\0');
		auto status = Win32::MultiByteToWideChar(
			Win32::CpUtf8,									// CodePage
			0,											// dwFlags
			str.data(),									// lpMultiByteStr
			static_cast<int>(str.size() * sizeof(char)),	// cbMultiByte
			wstrTo.data(),									// lpWideCharStr
			static_cast<int>(wstrTo.size())				// cchWideChar
		);
		if (status == 0)
			throw Error::Win32Error{Win32::GetLastError(), "MultiByteToWideChar() [2] failed"};

		return wstrTo;
	}
}

export namespace RAII
{
	template<auto VDeleteFn>
	struct Deleter
	{
		static constexpr void operator()(auto handle) noexcept
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
	using HkeyUniquePtr = IndirectUniquePtr<Win32::HKEY, Win32::RegCloseKey>;
}

export namespace Registry
{
	auto GetString(
		Win32::HKEY hKey,
		const std::wstring& subKey,
		const std::wstring& value
	) -> std::wstring
	{
		auto dataSize = Win32::DWORD{};
		auto retCode = Win32::RegGetValueW(
			hKey,
			subKey.c_str(),
			value.c_str(),
			Win32::RrfRtRegSz,
			nullptr,
			nullptr,
			&dataSize
		);
		if (retCode != 0)
			throw Error::Win32Error{ static_cast<Win32::DWORD>(retCode), "Cannot read string from registry" };

		auto data = std::wstring{};
		data.resize(dataSize / sizeof(wchar_t));

		retCode = Win32::RegGetValueW(
			hKey,
			subKey.c_str(),
			value.c_str(),
			Win32::RrfRtRegSz,
			nullptr,
			data.data(),
			&dataSize
		);
		if (retCode != 0)
			throw Error::Win32Error{ static_cast<Win32::DWORD>(retCode), "Cannot read string from registry" };

		auto stringLengthInWchars = Win32::DWORD{dataSize / sizeof(wchar_t)};
		stringLengthInWchars--; // Exclude the NUL written by the Win32 API
		data.resize(stringLengthInWchars);

		return data;
	}
}

export namespace Security
{
	// https://learn.microsoft.com/en-us/windows/win32/secauthz/enabling-and-disabling-privileges-in-c--
	auto SetPrivilege(
		Win32::HANDLE hToken,          // access token handle
		Win32::LPCWSTR lpszPrivilege,  // name of privilege to enable/disable
		Win32::BOOL bEnablePrivilege   // to enable or disable privilege
	) -> Win32::BOOL
	{
		auto luid = Win32::LUID{};

		auto success = Win32::LookupPrivilegeValueW(
			nullptr,            // lookup privilege on local system
			lpszPrivilege,   // privilege to lookup 
			&luid // receives LUID of privilege
		);        
		if (not success)
			throw Error::Win32Error{Win32::GetLastError(), "LookupPrivilegeValue error"};
		
		auto tp = Win32::TOKEN_PRIVILEGES{
			.PrivilegeCount = 1,
			.Privileges = {
				{
					.Luid = luid,
					.Attributes = bEnablePrivilege ? Win32::SePrivilegeEnabled : 0ul
				}
			}
		};

		// Enable the privilege or disable all privileges.
		success = Win32::AdjustTokenPrivileges(
			hToken,
			false,
			&tp,
			sizeof(Win32::TOKEN_PRIVILEGES),
			nullptr,
			nullptr
		);
		if (not success)
			throw Error::Win32Error{Win32::GetLastError(), "AdjustTokenPrivileges error"};

		if (Win32::GetLastError() == Win32::ErrorNotAllAssigned)
			return false;

		return true;
	}
}