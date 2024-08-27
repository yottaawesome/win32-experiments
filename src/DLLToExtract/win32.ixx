module;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

export module win32;

export namespace Win32
{
    using 
        ::BOOL,
        ::HMODULE,
        ::LPVOID,
        ::DWORD
        ;

    enum class DLLEntryReason
    {
        ProcessAttach = DLL_PROCESS_ATTACH,
        ThreadAttach = DLL_THREAD_ATTACH,
        ThreadDetach = DLL_THREAD_DETACH,
        ProcessDetach = DLL_PROCESS_DETACH
    };
}
