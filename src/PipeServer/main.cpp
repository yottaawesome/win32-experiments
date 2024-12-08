import pipelib;
import win32;
import std;

auto main() -> int
try
{
    PipeLib::UniqueHandle serverPipe = PipeLib::OpenServerPipe();

    PipeLib::Overlapped serverConnected{};
    Win32::BOOL connStatus = Win32::ConnectNamedPipe(serverPipe.get(), &serverConnected);
    if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::IoPending)
        throw PipeLib::Win32Error(lastError, "ConnectNamedPipe() failed");

    PipeLib::UniqueHandle clientPipe = PipeLib::OpenClientPipe();
    if (serverConnected.Wait(Win32::Infinite))
        std::println("Connection wait was successful.");

    std::string msg = "Hello, world!";
    std::vector<std::byte> data = msg
        | std::ranges::views::transform([](char c) { return static_cast<std::byte>(c); })
        | std::ranges::to<std::vector<std::byte>>();
    if constexpr (false)
    {
        PipeLib::WritePipe(serverPipe.get(), data);
        std::vector<std::byte> readData = PipeLib::ReadPipe(clientPipe.get());
        std::string readString{ reinterpret_cast<char*>(readData.data()), readData.size() };
        std::println("Read back {}", readString);
    }
    else
    {
        PipeLib::WriteWithHeader(serverPipe.get(), data);
        std::pair readOperation = PipeLib::ReadWithHeader(clientPipe.get());
        std::string readString{ reinterpret_cast<char*>(readOperation.second.data()), readOperation.second.size() };
        std::println(
R"(Read message:
 -> Header type {}
 -> subtype {}
 -> data: {})",
            static_cast<int>(readOperation.first.Type),
            static_cast<int>(readOperation.first.Subtype),
            readString
        );
    }

    return 0;
}
catch (const std::exception& ex)
{
    std::println("{}", ex.what());
    return 1;
}
