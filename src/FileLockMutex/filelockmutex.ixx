#pragma comment(lib, "Pathcch.lib")

export module filelockmutex;
export import :win32;
export import :filesystemlock;
import std;

auto StartChildProcess() -> Process::ProcessInformation
{
	auto startupInfo = Process::StartupInfo{};
	auto processInfo = Process::ProcessInformation{};
	auto exePath = Process::GetCurrentExecutablePath();
	auto cmd = std::wstring{ L"FileLockMutex.exe hello" };
	bool succeeded = Win32::CreateProcessW(
		exePath.wstring().data(),
		cmd.data(),
		nullptr,
		nullptr,
		false,
		0,
		nullptr,
		nullptr,
		&startupInfo,
		&processInfo
	);
	if (not succeeded)
	{
		const auto lastError = Win32::GetLastError();
		throw Error::Win32Error(lastError, "Failed to create process.");
	}
	std::println("Parent: succeeded creating process.");
	return processInfo;
}

auto ChildProcessMain(int args, char* argv[]) -> int
{
	std::println("Child: process started successfully.");
	Async::FileSystemLock<> fileLock{ false };
	std::println("Child: file lock attempt succeeded: {}", fileLock.try_lock());
	return 0;
}

auto ParentProcessMain(int args, char* argv[]) -> int
{
	Async::FileSystemLock<> fileLock{ true };
	auto processInfo = StartChildProcess();

	auto result = Win32::WaitForSingleObject(processInfo.hProcess, Win32::Infinite);
	if (result != Win32::WaitResult::Object0)
	{
		const auto lastError = Win32::GetLastError();
		throw Error::Win32Error(lastError, "Failed to wait for process.");
	}
	return 0;
}

extern "C" auto main(int args, char* argv[]) -> int
try
{
	if (args == 2)
		return ChildProcessMain(args, argv);
	else
		return ParentProcessMain(args, argv);
}
catch (const std::exception& ex)
{
	std::println("Exception in main: {}.", ex.what());
	return -1;
}
