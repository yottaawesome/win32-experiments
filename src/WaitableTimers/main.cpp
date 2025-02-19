import std;
import win32;

namespace Error
{
	auto TranslateError(Win32::DWORD code) noexcept -> std::string
	{
		void* buffer = nullptr;
		Win32::DWORD count = Win32::FormatMessageA(
			Win32::FormatFlags::AllocateBuffer | Win32::FormatFlags::FromSystem | Win32::FormatFlags::IgnoreInserts,
			nullptr,
			code,
			0,
			reinterpret_cast<Win32::LPSTR>(&buffer),
			0,
			nullptr
		);
		if (not buffer)
			return std::format("Failed interpreting {}", code);
		std::string message(reinterpret_cast<char*>(buffer));
		Win32::LocalFree(buffer);
		return message;
	}

	struct Win32Error : std::runtime_error
	{
		Win32Error(Win32::DWORD code, std::string_view msg)
			: runtime_error(std::format("{} -> {} ({})", msg, TranslateError(code), code))
		{ }
	};
}

namespace RAII
{
	template<auto VDeleteFn>
	struct Deleter
	{
		void operator()(auto handle) const noexcept
		{
			VDeleteFn(handle);
		}
	};
	template<typename THandle, auto VDeleteFn>
	using UniquePtr = std::unique_ptr<THandle, Deleter<VDeleteFn>>;
	template<typename THandle, auto VDeleteFn>
	using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<THandle>, Deleter<VDeleteFn>>;

	using HandleUniquePtr = IndirectUniquePtr<Win32::HANDLE, Win32::CloseHandle>;
}

#pragma region BasicExample
namespace BasicExample
{
	void Run()
	{
		// https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createwaitabletimerw
		Win32::HANDLE hTimer = Win32::CreateWaitableTimerW(
			nullptr,
			false,		// Not a manual-reset timer.
			nullptr
		);
		if (not hTimer)
			throw Error::Win32Error(Win32::GetLastError(), "Failed creating timer.");

		RAII::HandleUniquePtr timer(hTimer);

		// https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-setwaitabletimer
		Win32::LARGE_INTEGER time{ .QuadPart = -1000 };
		Win32::BOOL success = Win32::SetWaitableTimer(
			timer.get(),
			&time,
			0,			// No period.
			nullptr,
			nullptr,
			false
		);
		if (not success)
			throw Error::Win32Error(Win32::GetLastError(), "Failed setting timer.");

		Win32::DWORD waitResult = Win32::WaitForSingleObject(timer.get(), Win32::Infinite);
		switch (waitResult)
		{
			case Win32::WaitConstants::Object0:
				break;
			case Win32::WaitConstants::Abandoned:
				throw std::runtime_error("The wait was unexpectedly abandoned.");
				break;
			case Win32::WaitConstants::Timeout:
				throw std::runtime_error("The wait unexpectedly timed out.");
				break;
			case Win32::WaitConstants::Failed:
				throw std::runtime_error("The wait unexpectedly failed.");
				break;
		}

		std::println("Success!");
	}
}
#pragma endregion BasicExample

auto main() -> int
try
{
	BasicExample::Run();

	return 0;
}
catch (const std::exception& ex)
{
	std::println("Failed: {}", ex.what());
	return 1;
}
