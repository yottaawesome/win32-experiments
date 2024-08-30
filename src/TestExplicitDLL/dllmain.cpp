import common;

Win32::BOOL __stdcall DllMain(
    Win32::HMODULE hModule,
    Win32::DWORD  ul_reason_for_call,
    Win32::LPVOID lpReserved
)
{
    switch (static_cast<Win32::DLLAttach>(ul_reason_for_call))
    {
        case Win32::DLLAttach::ProcessAttach:
        case Win32::DLLAttach::ThreadAttach:
        case Win32::DLLAttach::ThreadDetach:
        case Win32::DLLAttach::ProcessDetach:
        break;
    }
    return true;
}

