import std;
import win32;

struct HandleDeleter
{
    void operator()(Win32::HANDLE h) const noexcept
    {
        if (h and h != Win32::InvalidHandleValue)
            Win32::CloseHandle(h);
    }
};
using UniqueHandle = std::unique_ptr<std::remove_pointer_t<Win32::HANDLE>, HandleDeleter>;

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

struct Overlapped : Win32::OVERLAPPED
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

namespace PipeOperations
{
    constexpr auto BufferSize = 1024;
    constexpr std::wstring_view pipeName = LR"(\\.\pipe\thynamedpipe)";
    
    auto OpenServerPipe() -> UniqueHandle
    {
        Win32::HANDLE serverPipe =
            Win32::CreateNamedPipeW(
                pipeName.data(),             // pipe name 
                Win32::Pipes::OpenMode::Mode::Duplex | Win32::Pipes::OpenMode::Flags::Overlapped,       // read/write access 
                Win32::Pipes::PipeMode::Type::Message | Win32::Pipes::PipeMode::Read::Message | Win32::Pipes::PipeMode::Wait::NoWait,      // message type pipe 
                Win32::Pipes::UnlimitedInstances, // max. instances  
                BufferSize,                  // output buffer size 
                BufferSize,                  // input buffer size 
                0,                        // client time-out 
                nullptr
            );
        if (auto lastError = Win32::GetLastError(); not serverPipe or serverPipe == Win32::InvalidHandleValue)
            throw Win32Error(lastError, "CreateNamedPipeW() failed");
        return UniqueHandle(serverPipe);
    }

    auto OpenClientPipe() -> UniqueHandle
    {
        Win32::HANDLE clientPipe = 
            Win32::CreateFileW(
                pipeName.data(),   // pipe name 
                Win32::AccessRights::GenericRead | Win32::AccessRights::GenericWrite,
                0,              // no sharing 
                nullptr,           // default security attributes
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

    auto ReadPipe(Win32::HANDLE pipe) -> std::vector<std::byte>
    {
        std::vector<std::byte> returnBuffer;
        Win32::DWORD totalBytes = 0;
        while (true)
        {
            std::vector<std::byte> buffer{ BufferSize };
            Win32::DWORD bytesRead = 0;
            Win32::BOOL success = Win32::ReadFile(
                pipe,    // pipe handle 
                buffer.data(),    // buffer to receive reply 
                static_cast<Win32::DWORD>(buffer.size()),  // size of buffer 
                &bytesRead,  // number of bytes read 
                nullptr// not overlapped 
            );
            if (bytesRead)
            {
                totalBytes += bytesRead;
                buffer.resize(bytesRead);
                returnBuffer.insert(returnBuffer.end(), buffer.begin(), buffer.end());
            }
            if (success)
                return returnBuffer;
            if (Win32::DWORD lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::MoreData)
                throw Win32Error(lastError, "ReadFile() failed.");
        }
    }

    auto WritePipe(Win32::HANDLE pipe, const std::vector<std::byte>& data)
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
}

auto main() -> int
try
{
    UniqueHandle serverPipe = PipeOperations::OpenServerPipe();

    Overlapped serverConnected{};
    Win32::BOOL connStatus = Win32::ConnectNamedPipe(serverPipe.get(), &serverConnected);
    if(auto lastError = GetLastError(); lastError != Win32::ErrorCodes::PipeListening)
        throw Win32Error(lastError, "ConnectNamedPipe() failed");

    UniqueHandle clientPipe = PipeOperations::OpenClientPipe();
    //if (serverConnected.Wait(Win32::Infinite))
    //    std::println("Wait was successful.");

    std::string msg = "Hello, world!";
    std::vector<std::byte> data = 
        msg 
        | std::ranges::views::transform([](char c) {return std::byte{ (std::byte)c }; })
        | std::ranges::to<std::vector<std::byte>>();
    PipeOperations::WritePipe(serverPipe.get(), data);

    std::vector<std::byte> readData = PipeOperations::ReadPipe(clientPipe.get());
    std::string readString{ reinterpret_cast<char*>(readData.data()), readData.size() };

    std::println("Read back {}", readString);

    return 0;
}
catch (const std::exception& ex)
{
    std::println("{}", ex.what());
    return 1;
}