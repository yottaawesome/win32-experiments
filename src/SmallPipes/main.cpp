import std;
import win32;

namespace RAII
{
	template<auto VFn>
	struct Deleter
	{
		void operator()(auto ptr)
		{
			VFn(ptr);
		}
	};
	template<typename T, auto VFn>
	using UniquePtr = std::unique_ptr<T, Deleter<VFn>>;
	template<typename T, auto VFn>
	using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<T>, Deleter<VFn>>;

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
			const auto lastError = Win32::GetLastError();
			return std::format("FormatMessageA() failed on code {} with error {}", errorCode, lastError);
		}

		std::string msg(static_cast<char*>(messageBuffer));
		// This should never happen
		// See also https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-raisefailfastexception
		if (Win32::LocalFree(messageBuffer))
			std::abort();

		std::erase_if(msg, [](const char x) { return x == '\n' || x == '\r'; });
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

namespace Async
{
	struct Overlapped : Win32::OVERLAPPED
	{
		~Overlapped()
		{
			Close();
		}

		Overlapped(const Overlapped&) = delete;
		Overlapped& operator=(const Overlapped&) = delete;

		Overlapped(Overlapped&& other)
		{
			Move(other);
		};

		Overlapped& operator=(Overlapped&& other)
		{
			Move(other);
		};

		Overlapped()
		{
			hEvent = Win32::CreateEventW(nullptr, true, false, nullptr);
			if (auto lastError = Win32::GetLastError(); not hEvent)
				throw Error::Win32Error(lastError, "CreateEventW() failed");
		}

		bool Wait(Win32::DWORD millis)
		{
			if (not hEvent)
				throw std::runtime_error("No event");
			auto waitStatus = Win32::WaitForSingleObject(hEvent, millis);
			return waitStatus == Win32::WaitObject0;
		}

	private:
		void Move(Overlapped& other)
		{
			Close();
			hEvent = other.hEvent;
			other.hEvent = nullptr;
		}

		void Close()
		{
			if (hEvent)
			{
				Win32::CloseHandle(hEvent);
				hEvent = nullptr;
			}
		}
	};

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

namespace SmallPipes
{
	constexpr auto BufferSize = 1024;
	constexpr auto PipeSize = BufferSize;
	constexpr std::wstring_view PipeName = LR"(\\.\pipe\thynamedpipe)";

	RAII::HandleUniquePtr CreateServerPipe()
	{
		Win32::HANDLE serverPipe = Win32::CreateNamedPipeW(
			PipeName.data(),             // pipe name 
			Win32::Pipes::OpenMode::Mode::Duplex | Win32::Pipes::OpenMode::Flags::Overlapped,       // read/write access 
			Win32::Pipes::PipeMode::Type::Message | Win32::Pipes::PipeMode::Read::Message,      // message type pipe 
			Win32::Pipes::UnlimitedInstances, // max. instances  
			PipeSize,                  // output buffer size 
			PipeSize,                  // input buffer size 
			0,                        // client time-out 
			nullptr
		);
		if (auto lastError = Win32::GetLastError(); not serverPipe or serverPipe == Win32::InvalidHandleValue)
			throw Error::Win32Error(lastError, "CreateNamedPipeW() failed");
		return RAII::HandleUniquePtr(serverPipe);
	}

	RAII::HandleUniquePtr CreateClientPipe()
	{
		Win32::HANDLE clientPipe = Win32::CreateFileW(
			PipeName.data(),   // pipe name 
			Win32::AccessRights::GenericRead | Win32::AccessRights::GenericWrite,
			0,              // no sharing 
			nullptr,           // default security attributes
			Win32::OpenExisting,  // opens existing pipe 
			0,              // default attributes 
			nullptr
		);
		if (auto lastError = Win32::GetLastError(); not clientPipe or clientPipe == Win32::InvalidHandleValue)
			throw Error::Win32Error(lastError, "CreateFile() failed");

		Win32::DWORD dwMode = Win32::Pipes::PipeMode::Read::Message;
		Win32::BOOL success = Win32::SetNamedPipeHandleState(
			clientPipe,    // pipe handle 
			&dwMode,  // new pipe mode 
			nullptr,     // don't set maximum bytes 
			nullptr
		);
		if (auto lastError = Win32::GetLastError(); not success)
			throw Error::Win32Error(lastError, "SetNamedPipeHandleState() failed");

		return RAII::HandleUniquePtr(clientPipe);
	}

	auto ReadPipe(Win32::HANDLE pipe) -> std::vector<std::byte>
	{
		std::vector<std::byte> returnBuffer{ BufferSize };
		Win32::DWORD totalBytesRead = 0;
		while (true)
		{
			Win32::DWORD bytesRead = 0;
			Win32::BOOL success = Win32::ReadFile(
				pipe,    // pipe handle 
				returnBuffer.data() + totalBytesRead,    // buffer to receive reply 
				BufferSize,  // size of buffer 
				&bytesRead,  // number of bytes read 
				nullptr// not overlapped 
			);
			totalBytesRead += bytesRead;
			if (success)
			{
				returnBuffer.resize(totalBytesRead);
				return returnBuffer;
			}
			if (Win32::DWORD lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::MoreData)
				throw Error::Win32Error(lastError, "ReadFile() failed.");
			returnBuffer.resize(returnBuffer.size() + BufferSize);
		}
	}

	struct Thread
	{
		Thread(Win32::HANDLE clientPipe)
			: m_clientPipe(clientPipe)
		{
			m_thread = std::jthread(
				[](Thread* t) { return t->Run(); }, 
				this
			);
		}
	private:
		auto Run() -> int
		{
			return 0;
		}

		std::jthread m_thread;
		Win32::HANDLE m_clientPipe = nullptr;
	};

	auto Run() -> int
	try
	{
		RAII::HandleUniquePtr server = CreateServerPipe();
		RAII::HandleUniquePtr client = CreateClientPipe();
		Thread t(client.get());

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
	return 0;
}
