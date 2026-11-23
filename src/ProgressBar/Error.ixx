export module MultiControlSamples:Error;
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

	struct ExactMessage
	{
		std::string Message;
	};

	class RuntimeError : public std::runtime_error
	{
	public:
		RuntimeError(const ExactMessage& exactMessage)
			: std::runtime_error{ exactMessage.Message }
		{
			Win32::OutputDebugStringA(what());
		}

		RuntimeError(const std::string& message, std::source_location location = std::source_location::current())
			: std::runtime_error{ FormatErrorMessage(message, location) }
		{
			Win32::OutputDebugStringA(what());
		}

	protected:
		[[nodiscard]]
		static auto FormatErrorMessage(const std::string& message, std::source_location location) -> std::string
		{
			return std::format("{} at {}:{}:{}", message, location.file_name(), location.line(), location.column());
		}
	};

	class Win32Error : public RuntimeError
	{
	public:
		Win32Error(Win32::DWORD errorCode, const std::string& message, std::source_location location = std::source_location::current())
			: RuntimeError{ ExactMessage{ FormatErrorMessage(errorCode, message, location) } }
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