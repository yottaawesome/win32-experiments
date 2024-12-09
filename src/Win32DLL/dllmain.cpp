import win32;

Win32::BOOL DllMain(
    Win32::HMODULE hModule,
    Win32::DWORD  ul_reason_for_call,
    Win32::LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case Win32::DllProcessAttach:
        Win32::OutputDebugStringA("Hello from the DLL!\n");
        break;
    case Win32::DllThreadAttach:
        Win32::OutputDebugStringA("Hello from the DLL!\n");
        break;
    case Win32::DllThreadDetach:
        Win32::OutputDebugStringA("Hello from the DLL!\n");
        break;
    case Win32::DllProcessDetach:
        Win32::OutputDebugStringA("Hello from the DLL!\n");
        break;
    }
    return true;
}

