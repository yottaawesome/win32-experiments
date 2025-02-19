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

	struct RuntimeError : std::runtime_error
	{
		template<typename...TArgs>
		RuntimeError(std::format_string<TArgs...> fmt, TArgs&&...args)
			: runtime_error(std::format(fmt, std::forward<TArgs>(args)...))
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

namespace TimerHelpers
{
	[[nodiscard]]
	auto MillisToHundredNanoIntervals(std::chrono::milliseconds ms) -> std::int64_t
	{
		return ms.count() * 10000;
	}

	auto SetTimer(Win32::HANDLE timer, std::chrono::milliseconds start, std::chrono::milliseconds period)
	{
		Win32::LARGE_INTEGER time{ .QuadPart = -MillisToHundredNanoIntervals(start) };
		Win32::BOOL success = Win32::SetWaitableTimer(
			timer,
			&time,
			static_cast<Win32::DWORD>(period.count()),
			nullptr,
			nullptr,
			false
		);
		if (not success)
			throw Error::Win32Error(Win32::GetLastError(), "Failed setting timer.");
	}

	[[nodiscard]]
	auto CreateTimer(bool manualReset) -> RAII::HandleUniquePtr
	{
		// https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createwaitabletimerw
		Win32::HANDLE hTimer = Win32::CreateWaitableTimerW(
			nullptr,
			manualReset,		// Not a manual-reset timer.
			nullptr
		);
		if (not hTimer)
			throw Error::Win32Error(Win32::GetLastError(), "Failed creating timer.");

		return RAII::HandleUniquePtr(hTimer);
	}

	[[nodiscard]]
	auto CreateEvent(bool manualReset, bool initialState) -> RAII::HandleUniquePtr
	{
		Win32::HANDLE exit = Win32::CreateEventW(nullptr, manualReset, initialState, nullptr);
		if (not exit)
			throw Error::Win32Error(Win32::GetLastError(), "CreateEventW() failed");
		return RAII::HandleUniquePtr(exit);
	}
}

namespace ThreadApc
{
	std::atomic<bool> KeepRunning = true;

	void PrintA(std::uint64_t)
	{
		std::println("A");
	}

	void PrintB(std::uint64_t)
	{
		std::println("B");
	}

	void PrintC(std::uint64_t)
	{
		std::println("C");
	}

	struct WorkerThread
	{
		~WorkerThread()
		{
			Win32::SetEvent(m_exit.get());
			Join();
		}

		void Start()
		{
			m_thread = std::jthread(&WorkerThread::Run, this);
		}

		void SignalToExit()
		{
			Win32::SetEvent(m_exit.get());
		}

		auto Join() -> bool
		{
			Win32::HANDLE hThread = m_thread.native_handle();
			if (not hThread)
				return true;

			Win32::DWORD result = Win32::WaitForSingleObject(hThread, Win32::Infinite);
			if (result == Win32::WaitConstants::Object0)
				return true;
			if (result == Win32::WaitConstants::Timeout)
				return false;
			throw Error::RuntimeError("WaitForSingleObject() returned {}", result);
		}

		auto Handle() -> Win32::HANDLE
		{
			return m_thread.native_handle();
		}

	private:
		void Run()
		{
			while (true)
			{
				Win32::DWORD result = Win32::WaitForSingleObjectEx(m_exit.get(), Win32::Infinite, true);

				if (result == Win32::WaitConstants::IoCompletion)
					;
				else if (result == Win32::WaitConstants::Object0)
				{
					std::println("Worker finished");
					return;
				}
				else
				{
					std::println("Unexpected wait result {}", result);
					return;
				}
			}
		}

		std::jthread m_thread;
		RAII::HandleUniquePtr m_exit = TimerHelpers::CreateEvent(false, false);
	};

	template<std::invocable T>
	struct Scope
	{
		~Scope() { m_t(); }
		Scope(T&& invocable) : m_t(invocable) {}
		T m_t;
	};

	void Run()
	try
	{
		auto timer = TimerHelpers::CreateTimer(false);

		WorkerThread worker;
		worker.Start();

		constexpr std::array Functions{ PrintA, PrintB, PrintC };
		TimerHelpers::SetTimer(timer.get(), std::chrono::milliseconds(1000), std::chrono::milliseconds(1000));

		for (size_t i = 0; i < Functions.size(); i++)
		{
			Win32::DWORD wait = Win32::WaitForSingleObjectEx(timer.get(), Win32::Infinite, true);
			if (wait != Win32::WaitConstants::Object0)
				throw Error::RuntimeError("Unexpected {}", wait);
			Win32::DWORD result = Win32::QueueUserAPC(
				Functions.at(i),
				worker.Handle(),
				0
			);
			if (not result)
				throw Error::Win32Error(Win32::GetLastError(), "QueueUserAPC() failed");
		}

		worker.SignalToExit();
		worker.Join();

		std::println("APC test all done.");
	}
	catch (const std::exception& ex)
	{
		std::println("Failed: {}", ex.what());
	}
}

auto main() -> int
try
{
	BasicExample::Run();
	ThreadApc::Run();

	return 0;
}
catch (const std::exception& ex)
{
	std::println("Failed: {}", ex.what());
	return 1;
}
