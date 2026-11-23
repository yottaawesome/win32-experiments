export module ProgressBarSamples:Error;
import std;
import Win32;

export namespace Error
{
	auto Win32CodeToString(Win32::DWORD code) -> std::string
	{
		auto buffer = static_cast<void*>(nullptr);
		auto chars =
			Win32::FormatMessageA(
				Win32::FormatMessageFlags::FromSystem | Win32::FormatMessageFlags::AllocateBuffer,
				nullptr,
				code,
				0,
				reinterpret_cast<Win32::LPSTR>(&buffer),
				0,
				nullptr
			);
		if (chars == 0 or not buffer)
			return std::format("Unknown error code: {}", code);

		auto message = std::string{ static_cast<char*>(buffer), chars };
		Win32::LocalFree(buffer);

		while (not message.empty() and (message.back() == '\r' or message.back() == '\n'))
			message.pop_back();
		return message;
	}

	class Win32Error : public std::runtime_error
	{
	public:
		Win32Error(Win32::DWORD errorCode, const std::string& message, std::source_location location = std::source_location::current())
			: std::runtime_error{ FormatErrorMessage(errorCode, message, location) }
			, m_errorCode{ errorCode }
		{ 
			Win32::OutputDebugStringA(what());
		}

		[[nodiscard]]
		auto GetErrorCode() const noexcept -> Win32::DWORD
		{
			return m_errorCode;
		}

	protected:
		[[nodiscard]]
		static auto FormatErrorMessage(
			Win32::DWORD errorCode, 
			const std::string& message,
			std::source_location location
		) -> std::string
		{
			return std::format("{} (Win32 Error Code: {}) at {}:{}:{}", message, errorCode, location.file_name(), location.line(), location.column());
		}

	protected:
		Win32::DWORD m_errorCode = 0;
	};
}