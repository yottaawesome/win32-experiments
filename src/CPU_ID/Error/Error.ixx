module;

#include <cstdio>
#include <ctime>

export module CpuId:Error;
import std;
import Win32;

export namespace Error
{
	[[noreturn]]
	void ErrorExit(
		int exitCode, 
		std::string_view message,
		const std::source_location& location = std::source_location::current()
	) noexcept
	{
		// Avoid heap-backed diagnostics, including STL debug bookkeeping.
		auto frames = std::array<Win32::LPVOID, 64>{};
		auto frameCount = Win32::RtlCaptureStackBackTrace(
			1, static_cast<Win32::DWORD>(frames.size()), frames.data(), nullptr);
		auto timestamp = std::array<char, 32>{};
		auto now = std::time(nullptr);
		auto utc = std::tm{};
		if (gmtime_s(&utc, &now) == 0)
			std::strftime(timestamp.data(), timestamp.size(), "%Y-%m-%dT%H:%M:%SZ", &utc);

		auto writeReport = [&](std::FILE* output) noexcept
		{
			std::fprintf(
				output,
				"\nFatal error at %s:%u\nFunction: %s\nExit code: %d\nMessage: ",
				location.file_name(),
				static_cast<unsigned>(location.line()),
				location.function_name(),
				exitCode);
			if (not message.empty())
				std::fwrite(message.data(), 1, message.size(), output);
			std::fputc('\n', output);
			std::fprintf(output, "Time: %s\nStacktrace:\n", timestamp.data());
			for (unsigned i = 0; i < frameCount; ++i)
			{
				std::fprintf(output, "%u> %p", i, frames[i]);
				auto module = Win32::HMODULE{};
				auto modulePath = std::array<wchar_t, 1024>{};
				if (Win32::GetModuleHandleExW(
					Win32::GetModuleHandleFlags::FromAddress | Win32::GetModuleHandleFlags::UnchangedRefCount,
					reinterpret_cast<const wchar_t*>(frames[i]),
					&module))
				{
					auto length = Win32::GetModuleFileNameW(
						module, modulePath.data(), static_cast<Win32::DWORD>(modulePath.size()));
					if (length > 0 and length < modulePath.size())
						std::fprintf(
							output, " (%ls+0x%llX)", modulePath.data(),
							static_cast<unsigned long long>(
								reinterpret_cast<std::uintptr_t>(frames[i]) - reinterpret_cast<std::uintptr_t>(module)));
				}
				std::fputc('\n', output);
			}
			std::fflush(output);
		};

		writeReport(stderr);

		auto logPath = std::array<wchar_t, 1024>{};
		constexpr auto logName = std::wstring_view{L"CPU_ID-fatal-error.log"};
		auto pathLength = Win32::GetTempPathW(static_cast<Win32::DWORD>(logPath.size()), logPath.data());
		if (pathLength > 0 and pathLength + logName.size() < logPath.size())
		{
			std::copy(logName.begin(), logName.end(), logPath.data() + pathLength);
			logPath[pathLength + logName.size()] = L'\0';
			auto log = static_cast<std::FILE*>(nullptr);
			if (_wfopen_s(&log, logPath.data(), L"a") == 0)
			{
				writeReport(log);
				std::fclose(log);
			}
		}

		// Fatal callback failures must not re-enter application code during teardown.
		std::_Exit(exitCode);
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