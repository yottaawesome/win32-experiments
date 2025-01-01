import std;
import win32;

namespace RAII
{
	template<auto VDeleteFn>
	struct Deleter
	{
		void operator()(auto ptr) const noexcept { VDeleteFn(ptr); }
	};
	template<typename T, auto VDeleteFn>
	using UniquePtr = std::unique_ptr<T, Deleter<VDeleteFn>>;
	template<typename T, auto VDeleteFn>
	using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<T>, Deleter<VDeleteFn>>;
	using HandleUniquePtr = IndirectUniquePtr<Win32::HANDLE, Win32::CloseHandle>;
}

namespace Error
{
	auto TranslateErrorCode(Win32::DWORD errorCode) -> std::string
	{
		constexpr Win32::DWORD flags =
			Win32::FormatMessageFlags::AllocateBuffer
			| Win32::FormatMessageFlags::FromSystem
			| Win32::FormatMessageFlags::IgnoreInserts;

		void* messageBuffer = nullptr;
		Win32::FormatMessageA(
			flags,
			nullptr,
			errorCode,
			0,
			reinterpret_cast<char*>(&messageBuffer),
			0,
			nullptr
		);
		if (not messageBuffer)
		{
			auto lastError = Win32::GetLastError();
			return std::format("FormatMessageA() failed on code {} with error {}", errorCode, lastError);
		}

		std::string msg(static_cast<char*>(messageBuffer));
		// This should never happen
		// See also https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-raisefailfastexception
		if (Win32::LocalFree(messageBuffer))
			std::abort();

		std::erase_if(msg, [](char x) noexcept { return x == '\n' or x == '\r'; });
		return msg;
	}

	struct Win32Error final : std::runtime_error
	{
		Win32Error(Win32::DWORD errorCode, std::string_view msg, std::source_location loc = std::source_location::current())
			: std::runtime_error(Format(errorCode, msg, loc))
		{ }

		static auto Format(Win32::DWORD errorCode, std::string_view msg, std::source_location loc) -> std::string
		{
			return std::format(
				"{}: {} ({}) -> {}:{}:{}",
				msg,
				TranslateErrorCode(errorCode),
				errorCode,
				loc.function_name(),
				loc.file_name(),
				loc.line()
			);
		}
	};
}

namespace Util
{
	auto StringToVector(std::string_view string) -> std::vector<std::byte>
	{
		if constexpr (true) return std::vector<std::byte>(
			reinterpret_cast<const std::byte*>(string.data()), 
			reinterpret_cast<const std::byte*>(string.data() + string.size())
		);
		else return string 
			| std::ranges::views::transform([](char c) { return static_cast<std::byte>(c); }) 
			| std::ranges::to<std::vector<std::byte>>();
	}

	auto VectorToString(const std::vector<std::byte>& vec) -> std::string
	{
		return std::string(
			reinterpret_cast<const char*>(vec.data()), 
			vec.size()
		);
	}
}

namespace Async
{
	struct [[nodiscard]] Overlapped : Win32::OVERLAPPED
	{
		~Overlapped() { Close(); }

		Overlapped(const Overlapped&) = delete;
		Overlapped& operator=(const Overlapped&) = delete;

		Overlapped(Overlapped&& other) { std::swap(*this, other); };
		Overlapped& operator=(Overlapped&& other) { std::swap(*this, other); };

		Overlapped() : Win32::OVERLAPPED{}
		{
			hEvent = Win32::CreateEventW(nullptr, true, false, nullptr);
			if (not hEvent)
				throw Error::Win32Error(Win32::GetLastError(), "CreateEventW() failed");
		}

		bool Wait(std::string_view msg)
		{
			constexpr std::chrono::milliseconds wait{ 1000 };
			while (not Wait(static_cast<Win32::DWORD>(wait.count())))
				std::println("{}", msg);
			return true;
		}

		bool Wait()
		{
			return Wait(Win32::Infinite);
		}

		bool Wait(Win32::DWORD millis)
		{
			if (not hEvent)
				throw std::runtime_error("No event");
			auto waitStatus = Win32::WaitForSingleObject(hEvent, millis);
			return waitStatus == Win32::WaitObject0;
		}

		bool IsPartial() const
		{
			return this->Internal == 0x80000005L;// STATUS_BUFFER_OVERFLOW;
		}

		std::uint64_t GetBytesRead() const noexcept
		{
			return this->InternalHigh;
		}

		void swap(Overlapped& other) noexcept
		{
			Win32::HANDLE temp = hEvent;
			hEvent = other.hEvent;
			other.hEvent = temp;
		}

	private:
		void Close()
		{
			if (hEvent)
			{
				Win32::CloseHandle(hEvent);
				hEvent = nullptr;
			}
		}
	};
	static_assert(std::movable<Overlapped>);
	static_assert(not std::copyable<Overlapped>);
	static_assert(std::swappable<Overlapped>);

	struct Event
	{
		Event()
		{
			Win32::HANDLE hEvent = Win32::CreateEventW(nullptr, true, false, nullptr);
			if (not hEvent)
				throw Error::Win32Error(Win32::GetLastError(), "Failed creating event");
			m_handle = RAII::HandleUniquePtr(hEvent);
		}

		Win32::HANDLE Get() const noexcept
		{
			return m_handle.get();
		}

	private:
		RAII::HandleUniquePtr m_handle;
	};
}

namespace PipeOperations
{
	constexpr auto BufferSize = 2;
	constexpr auto PipeSize = BufferSize;
	constexpr std::wstring_view PipeName = LR"(\\.\pipe\thynamedpipe)";

	RAII::HandleUniquePtr CreateServerPipe()
	{
		Win32::HANDLE serverPipe = Win32::CreateNamedPipeW(
			PipeName.data(),
			Win32::Pipes::OpenMode::Mode::Duplex | Win32::Pipes::OpenMode::Flags::Overlapped,
			Win32::Pipes::PipeMode::Type::Message | Win32::Pipes::PipeMode::Read::Message,
			Win32::Pipes::UnlimitedInstances,
			PipeSize,
			PipeSize,
			0,
			nullptr
		);
		if (not serverPipe or serverPipe == Win32::InvalidHandleValue)
			throw Error::Win32Error(Win32::GetLastError(), "CreateNamedPipeW() failed");
		return RAII::HandleUniquePtr(serverPipe);
	}

	RAII::HandleUniquePtr CreateClientPipe()
	{
		Win32::HANDLE clientPipe = Win32::CreateFileW(
			PipeName.data(),
			Win32::AccessRights::GenericRead | Win32::AccessRights::GenericWrite,
			0,
			nullptr,
			Win32::OpenExisting,  
			0,
			nullptr
		);
		if (not clientPipe or clientPipe == Win32::InvalidHandleValue)
			throw Error::Win32Error(Win32::GetLastError(), "CreateFile() failed");

		Win32::DWORD dwMode = Win32::Pipes::PipeMode::Read::Message;
		Win32::BOOL success = Win32::SetNamedPipeHandleState(
			clientPipe,
			&dwMode,
			nullptr,
			nullptr
		);
		if (not success)
			throw Error::Win32Error(Win32::GetLastError(), "SetNamedPipeHandleState() failed");

		return RAII::HandleUniquePtr(clientPipe);
	}
}

namespace PipeOperations::Blocking
{
	auto WritePipe(Win32::HANDLE pipe, const std::vector<std::byte>& data)
	{
		Win32::DWORD bytesWritten = 0;
		Win32::BOOL success = Win32::WriteFile(
			pipe,
			data.data(),
			static_cast<Win32::DWORD>(data.size()),
			&bytesWritten,
			nullptr
		);
		if (not success)
			throw Error::Win32Error(Win32::GetLastError(), "WriteFile() failed.");
	}

	auto ReadPipe(Win32::HANDLE pipe, Win32::DWORD bytesToRead) -> std::vector<std::byte>
	{
		std::vector<std::byte> returnBuffer{ bytesToRead };
		std::uint64_t totalBytesRead = 0;
		while (true)
		{
			Win32::DWORD bytesRead = 0;
			Win32::BOOL success = Win32::ReadFile(
				pipe,
				returnBuffer.data() + totalBytesRead,
				bytesToRead,
				&bytesRead,
				nullptr
			);
			totalBytesRead += bytesRead;
			if (success)
			{
				returnBuffer.resize(totalBytesRead);
				return returnBuffer;
			}
			if (Win32::DWORD lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::MoreData)
				throw Error::Win32Error(lastError, "ReadFile() failed.");
			returnBuffer.resize(returnBuffer.size() + bytesToRead);
		}
	}
}

namespace PipeOperations::Nonblocking
{
	// If more data is written than the buffer can hold,
	// the write operation will block until the data is 
	// read.
	auto WritePipe(Win32::HANDLE pipe, const std::vector<std::byte>& data)
	{
		Win32::DWORD bytesWritten = 0;
		Async::Overlapped overlapped;
		Win32::BOOL success = Win32::WriteFile(
			pipe,
			data.data(),
			static_cast<Win32::DWORD>(data.size()),
			&bytesWritten,
			&overlapped
		);
		if (not success and Win32::GetLastError() != Win32::ErrorCodes::IoPending)
			throw Error::Win32Error(Win32::GetLastError(), "WriteFile() failed.");
		overlapped.Wait("Still waiting...");
	}

	// If the buffer is too small to hold all data,
	// multiple reads are required.
	auto ReadPipe(Win32::HANDLE pipe, Win32::DWORD bytesToRead) -> std::vector<std::byte>
	{
		std::vector<std::byte> returnBuffer{ bytesToRead };
		std::uint64_t totalBytesRead = 0;
		while (true)
		{
			Win32::DWORD bytesRead = 0;
			Async::Overlapped overlapped;
			Win32::BOOL success = Win32::ReadFile(
				pipe,
				returnBuffer.data() + totalBytesRead,
				bytesToRead,
				&bytesRead, // not used for async IO
				&overlapped
			);
			overlapped.Wait("Waiting to do read...");
			totalBytesRead += overlapped.GetBytesRead();
			if (success)
			{
				returnBuffer.resize(totalBytesRead);
				return returnBuffer;
			}
			if (Win32::DWORD lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::MoreData)
				throw Error::Win32Error(lastError, "ReadFile() failed.");
			returnBuffer.resize(returnBuffer.size() + bytesToRead);
		}
	}
}

namespace SmallPipes
{
	auto ServerProc(Win32::HANDLE pipe) -> int
	{
		constexpr std::string_view string = "ABC";
		std::println("Writing string {}", string);
		PipeOperations::Nonblocking::WritePipe(pipe, Util::StringToVector(string));
		return 0;
	};

	auto ClientProc(Win32::HANDLE pipe) -> int
	{
		std::vector<std::byte> bytes = PipeOperations::Nonblocking::ReadPipe(pipe, PipeOperations::BufferSize);
		std::string string = Util::VectorToString(bytes);
		std::println("Read string {}", string);
		return 0;
	};

	auto Run() -> int
	try
	{
		RAII::HandleUniquePtr serverEnd = PipeOperations::CreateServerPipe();
		RAII::HandleUniquePtr clientEnd = PipeOperations::CreateClientPipe();

		constexpr auto ComposeThread = 
			[](Win32::HANDLE pipe, std::string_view name, auto proc) constexpr -> auto
			{
				return [pipe, name, proc]
				{
					try
					{
						proc(pipe);
					}
					catch (const std::exception& ex)
					{
						std::println("Error in {}: {}", name, ex.what());
					}
				};
			};

		std::jthread serverThread(ComposeThread(serverEnd.get(), "Server", ServerProc));
		std::jthread clientThread(ComposeThread(clientEnd.get(), "Client", ClientProc));

		serverThread.join();
		clientThread.join();

		std::println("Successful.");

		return 0;
	}
	catch (const std::exception& ex)
	{
		std::println("Exception: {}", ex.what());
		return 1;
	}
	catch (...)
	{
		std::println("Unknown error");
		return 1;
	}
}

auto main() -> int
{
	SmallPipes::Run();
	return 0;
}
