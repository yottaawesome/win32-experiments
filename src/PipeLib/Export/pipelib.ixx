export module pipelib;
import stdlib;
import win32;

#pragma warning(disable: 4251)
#define EXPORT __declspec(dllexport)

export namespace PipeLib
{
	struct EXPORT HandleDeleter
	{
		void operator()(Win32::HANDLE h) const noexcept
		{
			if (h and h != Win32::InvalidHandleValue)
				Win32::CloseHandle(h);
		}
	};
	using UniqueHandle = std::unique_ptr<std::remove_pointer_t<Win32::HANDLE>, HandleDeleter>;

    EXPORT auto TranslateErrorCode(Win32::DWORD errorCode) -> std::string
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

#pragma warning(suppress: 4275)
    struct EXPORT Win32Error final : std::runtime_error
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

    struct EXPORT Overlapped : Win32::OVERLAPPED
    {
        Overlapped()
        {
            hEvent = Win32::CreateEventW(nullptr, true, false, nullptr);
            if (auto lastError = Win32::GetLastError(); not hEvent)
                throw Win32Error(lastError, "CreateEventW() failed");
            ServerConnected = UniqueHandle(hEvent);
        }

        bool Wait(Win32::DWORD millis)
        {
            auto waitStatus = Win32::WaitForSingleObject(ServerConnected.get(), millis);
            return waitStatus == Win32::WaitObject0;
        }

        UniqueHandle ServerConnected{};
    };

    constexpr auto EXPORT BufferSize = 1024;
    constexpr auto EXPORT PipeSize = BufferSize;
    constexpr std::wstring_view EXPORT pipeName = LR"(\\.\pipe\thynamedpipe)";

    EXPORT auto OpenServerPipe() -> UniqueHandle
    {
        Win32::HANDLE serverPipe = Win32::CreateNamedPipeW(
            pipeName.data(),             // pipe name 
            Win32::Pipes::OpenMode::Mode::Duplex | Win32::Pipes::OpenMode::Flags::Overlapped,       // read/write access 
            Win32::Pipes::PipeMode::Type::Message | Win32::Pipes::PipeMode::Read::Message,      // message type pipe 
            Win32::Pipes::UnlimitedInstances, // max. instances  
            PipeSize,                  // output buffer size 
            PipeSize,                  // input buffer size 
            0,                        // client time-out 
            nullptr
        );
        if (auto lastError = Win32::GetLastError(); not serverPipe or serverPipe == Win32::InvalidHandleValue)
            throw Win32Error(lastError, "CreateNamedPipeW() failed");
        return UniqueHandle(serverPipe);
    }

    EXPORT auto OpenClientPipe() -> UniqueHandle
    {
        Win32::SECURITY_ATTRIBUTES security{
            .nLength = sizeof(Win32::SECURITY_ATTRIBUTES),
            .bInheritHandle = true
        };

        Win32::HANDLE clientPipe = Win32::CreateFileW(
            pipeName.data(),   // pipe name 
            Win32::AccessRights::GenericRead | Win32::AccessRights::GenericWrite,
            0,              // no sharing 
            &security,           // security attributes
            Win32::OpenExisting,  // opens existing pipe 
            0,              // default attributes 
            nullptr
        );
        if (auto lastError = Win32::GetLastError(); not clientPipe or clientPipe == Win32::InvalidHandleValue)
            throw Win32Error(lastError, "CreateFile() failed");

        Win32::DWORD dwMode = Win32::Pipes::PipeMode::Read::Message;
        Win32::BOOL success = Win32::SetNamedPipeHandleState(
            clientPipe,    // pipe handle 
            &dwMode,  // new pipe mode 
            nullptr,     // don't set maximum bytes 
            nullptr
        );
        if (auto lastError = Win32::GetLastError(); not success)
            throw Win32Error(lastError, "SetNamedPipeHandleState() failed");

        return UniqueHandle(clientPipe);
    }

    enum class EXPORT MessageType
    {
        Unset,
        TypeA,
        TypeB,
    };
    enum class EXPORT Subtype
    {
        Unset,
        TypeA,
        TypeB,
    };
    struct EXPORT Header
    {
        MessageType Type = MessageType::Unset;
        Subtype Subtype = Subtype::Unset;
    };

    EXPORT auto ReadPipe(Win32::HANDLE pipe) -> std::vector<std::byte>
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
                throw Win32Error(lastError, "ReadFile() failed.");
            returnBuffer.resize(returnBuffer.size() + BufferSize);
        }
    }

    EXPORT auto ReadWithHeader(Win32::HANDLE pipe) -> std::pair<Header, std::vector<std::byte>>
    {
        std::vector data = ReadPipe(pipe);
        if (data.size() < sizeof(Header))
            return {};
        return { *reinterpret_cast<Header*>(data.data()), { data.begin() + sizeof(Header), data.end()} };
    }

    EXPORT auto WritePipe(Win32::HANDLE pipe, const std::vector<std::byte>& data)
    {
        Win32::DWORD bytesWritten = 0;
        Win32::BOOL success = Win32::WriteFile(
            pipe,                  // pipe handle 
            data.data(),             // message 
            static_cast<Win32::DWORD>(data.size()),              // message length 
            &bytesWritten,             // bytes written 
            nullptr                 // not overlapped 
        );
        if (auto lastError = GetLastError(); not success)
            throw Win32Error(lastError, "WriteFile() failed.");
    }

    EXPORT auto WriteWithHeader(Win32::HANDLE pipe, const std::vector<std::byte>& data)
    {
        std::vector<std::byte> dataWithHeader(sizeof(Header) + data.size());
        Header* header = new (dataWithHeader.data()) Header{ .Type = MessageType::TypeB, .Subtype = Subtype::TypeA };
        dataWithHeader.insert(dataWithHeader.begin() + sizeof(Header), data.begin(), data.end());
        WritePipe(pipe, dataWithHeader);
    }

    struct EXPORT ProcessInfo : Win32::PROCESS_INFORMATION
    {
        ~ProcessInfo()
        {
            Close();
        }
        constexpr ProcessInfo() : PROCESS_INFORMATION{ 0 } {}

        ProcessInfo(const ProcessInfo&) = delete;
        auto operator=(const ProcessInfo&) -> ProcessInfo& = delete;
        ProcessInfo(ProcessInfo&&) noexcept = default;
        auto operator=(ProcessInfo&&) noexcept -> ProcessInfo& = default;

        private:
        auto Close() -> void
        {
            if (hThread)
                Win32::CloseHandle(hThread);
            if (hProcess)
                Win32::CloseHandle(hProcess);
            hThread = hProcess = nullptr;
        }

        auto Move(ProcessInfo& other) -> ProcessInfo&
        {
            Close();
            hThread = other.hThread;
            hProcess = other.hProcess;
            return *this;
        }

        auto ReleaseProcessHandle() -> Win32::HANDLE
        {
            Win32::HANDLE temp = hProcess;
            hProcess = nullptr;
            return temp;
        }

        auto ReleaseThreadHandle() -> Win32::HANDLE
        {
            Win32::HANDLE temp = hThread;
            hThread = nullptr;
            return temp;
        }
    };
}
