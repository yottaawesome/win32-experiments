#pragma comment(lib, "Comctl32.lib")

import std;
import CpuId;

auto wWinMain(Win32::HINSTANCE, Win32::HINSTANCE, Win32::LPWSTR, int) noexcept -> int
try
{
    return Application::App{}.Run();
}
catch (const std::exception& error)
{
    Error::ErrorExit(1, error.what());
}
catch (...)
{
    Error::ErrorExit(1, "Unknown exception during application startup or message loop");
}
