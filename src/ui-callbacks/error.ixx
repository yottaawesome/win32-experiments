export module uicallbacks:error;
import std;
import :win32;

export namespace UiCallbacks::Error
{
	auto FormatMessage(Win32::DWORD errorCode) -> std::string
	{
		auto messageBuffer = static_cast<Win32::LPSTR>(nullptr);
		auto size = Win32::FormatMessageA(
			Win32::FormatMessageFlags::FromSystem | Win32::FormatMessageFlags::AllocateBuffer,
			nullptr,
			errorCode,
			0,
			reinterpret_cast<Win32::LPSTR>(&messageBuffer),
			0,
			nullptr
		);
		if (size == 0)
			return "Unknown error";
		auto message = std::string{ messageBuffer, size };
		Win32::LocalFree(messageBuffer);
		return message;
	}

	class Error : public std::runtime_error
	{
	public:
		Error(std::string_view message) : std::runtime_error(std::string{message}) {}
	};

	class Win32Error : public std::runtime_error
	{
	public:
		Win32Error(Win32::DWORD errorCode, std::string_view message)
			: std::runtime_error(Format(errorCode, message)), errorCode(errorCode) {}

		constexpr auto Code(this const Win32Error& self) noexcept -> Win32::DWORD { return self.errorCode; }

	private:
		static auto Format(Win32::DWORD errorCode, std::string_view message) -> std::string
		{
			auto errorMessage = FormatMessage(errorCode);
			return std::format("{} -> {}", message, errorMessage);
		}

		Win32::DWORD errorCode = 0;
	};
}