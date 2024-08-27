import win32;

Win32::BOOL __stdcall DllMain(
    Win32::HMODULE hModule,
    Win32::DWORD  ul_reason_for_call,
    Win32::LPVOID lpReserved
)
{
    switch (static_cast<Win32::DLLEntryReason>(ul_reason_for_call))
    {
        case Win32::DLLEntryReason::ProcessAttach:
            break;
        case Win32::DLLEntryReason::ThreadAttach:
            break;
        case Win32::DLLEntryReason::ThreadDetach:
            break;
        case Win32::DLLEntryReason::ProcessDetach:
            break;
    }
    return true;
}

