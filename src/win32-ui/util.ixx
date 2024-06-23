export module util;
import std;
import shared;

export namespace Util
{
	template<typename T>
	concept ValidCharType = std::same_as<T, char> or std::same_as<T, wchar_t>;

	template<typename T>
	concept ValidViewType = std::same_as<T, std::string_view> or std::same_as<T, std::wstring_view>;

	template<typename T>
	concept ValidStringType = std::same_as<T, std::string> or std::same_as<T, std::wstring>;

	template <ValidCharType TChar, ValidViewType TView, ValidStringType TString, size_t N>
	struct FixedString
	{
		using CharType = TChar;
		using ViewType = TView;
		using StringType = TString;

		TChar buf[N]{};

		consteval FixedString(const TChar(&arg)[N]) noexcept
		{
			std::copy_n(arg, N, buf);
		}

		constexpr operator const TChar* () const noexcept
		{
			return buf;
		}

		consteval TView ToView() const noexcept
		{
			return { buf };
		}

		consteval operator TView() const noexcept
		{
			return { buf };
		}

		constexpr operator TString() const noexcept
		{
			return { buf };
		}

		constexpr TString ToString() const noexcept
		{
			return { buf };
		}
	};

	template<size_t N>
	using WideFixedString = FixedString<wchar_t, std::wstring_view, std::wstring, N>;
	template<size_t N>
	using NarrowFixedString = FixedString<char, std::string_view, std::string, N>;
}

export namespace Error
{
	template<Util::WideFixedString VModule = L"">
	std::string TranslateErrorCode(Win32::DWORD errorCode)
	{
		std::wstring_view moduleName = VModule;
		Win32::HMODULE moduleToSearch =
			moduleName.empty()
			? nullptr
			: Win32::GetModuleHandleW(moduleName.data());

		const Win32::DWORD flags =
			Win32::FormatMessageAllocateBuffer
			| Win32::FormatMessageFromSystem
			| Win32::FormatMessageIgnoreInserts
			| (moduleToSearch ? Win32::FormatMessageFromHModule : 0);

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