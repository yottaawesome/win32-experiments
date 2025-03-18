import std;
import win32;
import common;

#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Userenv")

namespace Log
{
	void LogToFile(std::string_view message)
	{
		std::ofstream log;
		log.open("C:\\temp\\child-log.txt", std::ios::app);
		log << message.data();
		log.close();
	}

	template<typename...TArgs>
	void Info(std::format_string<TArgs...> fmt, TArgs&&...args)
	{
		auto const time = std::chrono::current_zone()
			->to_local(std::chrono::system_clock::now());

		LogToFile(
			std::format(
				"[{}] {}\n",
				std::format("{:%Y-%m-%d %X}", time),
				std::format(fmt, std::forward<TArgs>(args)...)));
	}

	template<typename...TArgs>
	void Info(std::wformat_string<TArgs...> fmt, TArgs&&...args)
	{
		auto const time = std::chrono::current_zone()
			->to_local(std::chrono::system_clock::now());

		LogToFile(
			Utl::ConvertString(
				std::format(
					L"[{}] {}\n",
					std::format(L"{:%Y-%m-%d %X}", time),
					std::format(fmt, std::forward<TArgs>(args)...))));
	}
}

int main()
{
	Log::Info("Hello, world!");

	try
	{
		std::wstring value = Registry::GetString(
			Win32::HKCU,
			LR"(SOFTWARE\Mozilla\Firefox\Default Browser Agent)",
			L"CurrentDefault"
		);

		// Not necessary
		/*Win32::PROFILEINFOW profile{.dwSize= sizeof(Win32::PROFILEINFOW)};
		Win32::BOOL success = Win32::LoadUserProfileW(
			Win32::GetCurrentProcessToken(),
			&profile
		);
		if (not success)
			throw Error::Win32Error(Win32::GetLastError(), "Failed to load profile");*/

		Log::Info(L"Got {}", value);
		std::wcout << value << std::endl;
	}
	catch (const std::exception& ex)
	{
		Log::Info("Failed reading from registry: {}", ex.what());
	}

	return 0;
}
