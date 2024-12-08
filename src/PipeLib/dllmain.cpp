import win32;

Win32::BOOL __stdcall DllMain(Win32::HMODULE hModule, Win32::DWORD  reason, Win32::LPVOID lpReserved)
{
    switch (reason)
    {
        case Win32::DllProcessAttach:
        case Win32::DllThreadAttach:
        case Win32::DllThreadDetach:
        case Win32::DllProcessDetach:
            Win32::OutputDebugStringA("OK\n");
            break;
    }
    return true;
}

