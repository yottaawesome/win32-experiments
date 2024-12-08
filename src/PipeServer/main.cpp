import pipelib;
import stdlib;

// std::rethrow_exception(std::current_exception());

auto main() -> int
try
{
    auto start = std::chrono::high_resolution_clock::now();
    // Create server pipe.
    PipeLib::UniqueHandle serverPipe = PipeLib::OpenServerPipe();
    PipeLib::Overlapped serverConnected{};
    // Start a connection operation on the server pipe.
    Win32::BOOL connStatus = Win32::ConnectNamedPipe(serverPipe.get(), &serverConnected);
    if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::IoPending)
        throw PipeLib::Win32Error(lastError, "ConnectNamedPipe() failed");

    // Create the client pipe which will be inherited by the child process.
    PipeLib::UniqueHandle clientPipe = PipeLib::OpenClientPipe();
    if (serverConnected.Wait(Win32::Infinite))
        std::println("Connection wait was successful.");

    // Create the child process.
    std::wstring cmdLine = std::format(L"PipeClient.exe {}", clientPipe.get());
    PipeLib::ProcessInfo piProcInfo;
    Win32::STARTUPINFO siStartInfo{ .cb = sizeof(Win32::STARTUPINFO) };
    constexpr std::wstring_view clientExePath = 
        []() consteval -> std::wstring_view
        {
            if constexpr (IsDebug())
                return LR"(..\x64\Debug\PipeClient.exe)";
            else
                return LR"(..\x64\Release\PipeClient.exe)";
        }();
    bool success = Win32::CreateProcessW(
        clientExePath.data(),
        cmdLine.data(), // command line 
        nullptr,
        nullptr,
        true,           // handles are inherited 
        0,
        nullptr,
        nullptr,
        &siStartInfo,
        &piProcInfo
    );
    if (auto lastError = Win32::GetLastError(); not success)
        throw PipeLib::Win32Error(lastError, "CreateProcessW()");

    // Write to the server pipe -- the data will be read by the child process.
    std::string msg = "Hello, world!";
    std::vector<std::byte> data = msg
        | std::ranges::views::transform([](char c) { return static_cast<std::byte>(c); })
        | std::ranges::to<std::vector<std::byte>>();
    PipeLib::WriteWithHeader(serverPipe.get(), data);

    // Wait for child process to exit.
    Win32::DWORD waitResult = Win32::WaitForSingleObject(piProcInfo.hProcess, Win32::Infinite);
    if (waitResult == Win32::WaitObject0)
        std::println("All pipe operations were successful.");
    std::println("Done in {}.", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start));

    return 0;
}
catch (const std::exception& ex)
{
    std::println("{}", ex.what());
    return 1;
}
catch (...)
{
    std::println("Unknown exception");
    return 1;
}
