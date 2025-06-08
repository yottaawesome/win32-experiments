export module util;
import std;
import shared;

export namespace Util
{
	template<typename T>
	concept ValidCharType = std::same_as<T, char> or std::same_as<T, wchar_t>;

	template<size_t N, ValidCharType TChar>
	struct FixedString
	{
		TChar Buffer[N];

		constexpr FixedString(const TChar(&args)[N]) noexcept
		{
			std::copy_n(args, N, Buffer);
		}

		constexpr auto View() const noexcept
		{
			return std::basic_string_view<TChar, std::char_traits<TChar>>(Buffer);
		}

		constexpr auto String() const noexcept
		{
			return std::basic_string<TChar, std::char_traits<TChar>>(Buffer);
		}

		constexpr auto Data() const noexcept
		{
			return Buffer;
		}
	};
	template<size_t N>
	FixedString(char buffer[N]) -> FixedString<N, char>;
	template<size_t N>
	FixedString(wchar_t buffer[N]) -> FixedString<N, wchar_t>;

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
			throw std::runtime_error(std::format("WideCharToMultiByte() [1] failed", Win32::GetLastError()));

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
			throw std::runtime_error(std::format("WideCharToMultiByte() [2] failed", Win32::GetLastError()));

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
			throw std::runtime_error(std::format("MultiByteToWideChar() [1] failed", Win32::GetLastError()));

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
			throw std::runtime_error(std::format("MultiByteToWideChar() [2] failed", Win32::GetLastError()));

		return wstrTo;
	}
}

export namespace Error
{
	template<Util::FixedString VModule = L"">
	std::string TranslateErrorCode(Win32::DWORD errorCode)
	{
		std::wstring_view moduleName = VModule.View();
		Win32::HMODULE moduleToSearch =
			moduleName.empty()
			? nullptr
			: Win32::GetModuleHandleW(moduleName.data());

		const Win32::DWORD flags =
			Win32::FormatMessageFlags::AllocateBuffer
			| Win32::FormatMessageFlags::FromSystem
			| Win32::FormatMessageFlags::IgnoreInserts
			| (moduleToSearch ? Win32::FormatMessageFlags::FromHModule : 0);

		void* messageBuffer = nullptr;
		Win32::FormatMessageA(
			flags,
			moduleToSearch,
			errorCode,
			0,
			reinterpret_cast<char*>(&messageBuffer),
			0,
			nullptr
		);
		if (not messageBuffer)
		{
			const auto lastError = Win32::GetLastError();
			return std::format("FormatMessageA() failed on code {} with error {}", errorCode, lastError);
		}

		std::string msg(static_cast<char*>(messageBuffer));
		// This should never happen
		// See also https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-raisefailfastexception
		if (Win32::LocalFree(messageBuffer))
			Win32::__fastfail(Win32::FailFast::FatalExit);

		std::erase_if(msg, [](const char x) { return x == '\n' || x == '\r'; });
		return msg;
	}

	template<typename...TArgs>
	struct Win32Error final : public std::runtime_error
	{
		Win32Error(std::format_string<TArgs...> fmt, TArgs&&...args, const std::source_location loc = std::source_location::current())
			: std::runtime_error(
				std::format(
					"{}\n\tfunction: {}\n\tfile: {}:{}",
					std::format(fmt, std::forward<TArgs>(args)...),
					loc.function_name(),
					loc.file_name(),
					loc.line()))
		{}

		Win32Error(const DWORD errorCode, std::format_string<TArgs...> fmt, TArgs&&...args, const std::source_location loc = std::source_location::current())
			: m_code(errorCode),
			std::runtime_error(
				std::format(
					"{} -> {}\n\tfunction: {}\n\tfile: {}:{}",
					std::format(fmt, std::forward<TArgs>(args)...),
					TranslateErrorCode(errorCode),
					loc.function_name(),
					loc.file_name(),
					loc.line()))
		{}

		bool operator==(const Win32::DWORD code) const noexcept
		{
			return m_code == code;
		}

		Win32::DWORD Code() const noexcept
		{
			return m_code;
		}

		private:
		Win32::DWORD m_code = 0;
	};
	template<typename...Ts>
	Win32Error(Win32::DWORD, std::format_string<Ts...>, Ts&&...) -> Win32Error<Ts...>;
	/*template<typename...Ts>
	Win32Error(Win32::DWORD, std::string, Ts&&...) -> Win32Error<Ts...>;*/
	/*template<typename...Ts>
	Win32Error(const char*, Ts&&...) -> Win32Error<Ts...>;*/
	template<typename...Ts>
	Win32Error(std::format_string<Ts...>, Ts&&...) -> Win32Error<Ts...>;

	struct TestTranslator
	{
		static std::string Translate(DWORD x)
		{
			return "some error";
		}
	};

	struct Win32Translator
	{
		static std::string Translate(DWORD x)
		{
			return TranslateErrorCode(x);
		}
	};

	struct ExactMessage
	{
		explicit ExactMessage(std::string msg)
			: Message(std::move(msg)) {}
		std::string Message;
	};

	template<typename TException, typename TDummyUnique, typename TTranslator = void, typename...TArgs>
	struct Error : public TException
	{
		virtual ~Error() = default;

		//// Required for cases where we inherit from Error.
		Error(ExactMessage e)
			: TException(std::move(e.Message))
		{}

		Error(
			std::format_string<TArgs...> str,
			TArgs&&...args,
			const std::source_location loc = std::source_location::current()
		) requires std::same_as<TException, std::runtime_error>
			: TException(std::format(str, std::forward<TArgs>(args)...))
		{}

		Error(
			std::format_string<TArgs...> str,
			TArgs&&...args,
			const std::source_location loc = std::source_location::current()
		) requires (not std::same_as<TException, std::runtime_error>)
			: TException(ExactMessage(std::format(str, std::forward<TArgs>(args)...)))
		{}

		Error(
			unsigned long value,
			std::format_string<TArgs...> str,
			TArgs&&...args,
			const std::source_location loc = std::source_location::current()
		) requires (std::same_as<TException, std::runtime_error> and not std::same_as<void, TTranslator>)
			: TException(std::format("{} -> {}", TTranslator::Translate(value), std::format(str, std::forward<TArgs>(args)...)))
		{}

		Error(
			unsigned long value,
			std::format_string<TArgs...> str,
			TArgs&&...args,
			const std::source_location loc = std::source_location::current()
		) requires (not std::same_as<TException, std::runtime_error> and not std::same_as<void, TTranslator>)
			: TException(ExactMessage(std::format("{} -> {}", TTranslator::Translate(value), std::format(str, std::forward<TArgs>(args)...))))
		{}
	};
	template<typename TException, typename TDummyUnique, typename TTranslator, typename...Ts>
	Error(std::format_string<Ts...>, Ts&&...) -> Error<TException, TDummyUnique, TTranslator, Ts...>;
	template<typename TException, typename TDummyUnique, typename TTranslator, typename...Ts>
	Error(unsigned long, std::format_string<Ts...>, Ts&&...) -> Error<TException, TDummyUnique, TTranslator, Ts...>;

	template<typename...T>
	using RuntimeError = Error<std::runtime_error, struct BasicRuntimeError1, void, T...>;
	template<typename...T>
	using Win32Error2 = Error<std::runtime_error, struct BasicRuntimeError2, Win32Translator, T...>;

	//// Inherits from WinSockError
	template<typename...T>
	using WinSockError = Error<Win32Error2<T...>, struct BasicRuntimeError3, Win32Translator, T...>;
	template<typename...T>
	using WinSockError2 = Error<std::runtime_error, struct BasicRuntimeError4, Win32Translator, T...>;

	void Blah()
	{
		try
		{
			Win32Error b(0, "a {}", 1);
			Win32Error a(0, "a");
			WinSockError c(0, "a {}", 1);
			WinSockError d(0, "a");
			WinSockError e("a");
			WinSockError f("a {}", 1);
			WinSockError2 g(0, "a {}", 1);
			WinSockError2 h(0, "a");
			WinSockError2 i("a");
			WinSockError2 j("a {}", 1);
		}
		catch (...)
		{
		}
	}
}