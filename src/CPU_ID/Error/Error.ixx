export module CpuId:Error;
import std;
import Win32;

export namespace Error
{
	void ErrorExit(
		int exitCode, 
		const std::string& message, 
		const std::source_location& location = std::source_location::current()
	)
	{
		std::cerr << message << std::endl;
		std::exit(exitCode);
	}

	auto TranslateWin32Error(Win32::DWORD errorCode) -> std::string
	{
		auto messageBuffer = Win32::LPWSTR{};
		auto size = Win32::FormatMessageW(
			Win32::FormatMessageFlags::AllocateBuffer | Win32::FormatMessageFlags::FromSystem | Win32::FormatMessageFlags::IgnoreInserts,
			nullptr,
			errorCode,
			0,
			reinterpret_cast<Win32::LPWSTR>(&messageBuffer),
			0,
			nullptr);
		if (size == 0)
			return std::format("Unknown error code: {}", errorCode);
		auto message = std::wstring(messageBuffer, size);
		Win32::LocalFree(messageBuffer);
		while (not message.empty() and (message.back() == L'\n' or message.back() == L'\r'))
			message.pop_back();
		return std::string(message.begin(), message.end());
	}

	class Win32Error : public std::runtime_error
	{
	public:
		Win32Error(Win32::DWORD errorCode)
			: std::runtime_error(std::format("{}", TranslateWin32Error(errorCode)))
			, m_errorCode(errorCode)
		{}

		Win32Error(Win32::DWORD errorCode, const std::string& message)
			: std::runtime_error(std::format("{}: {}", message, TranslateWin32Error(errorCode)))
			, m_errorCode(errorCode)
		{}

		[[nodiscard]]
		constexpr auto ErrorCode() const noexcept -> Win32::DWORD
		{
			return m_errorCode;
		}
	private:
		Win32::DWORD m_errorCode = 0;
	};
}