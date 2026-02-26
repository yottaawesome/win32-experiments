module;

#include <iostream>
#include <csignal>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

export module CrashHandler;

#ifdef _WIN32
#include <windows.h>

export void BoltDownExceptionFilter() {
    // 1. Get the address of the function in kernel32.dll
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    if (!hKernel32) return;

    void* pAddr = (void*)GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
    if (!pAddr) return;

    // 2. Prepare the "patch" code. 
    // On x86/x64, we use 'ret' (Return) to exit the function immediately.
    // x86 (32-bit) uses 'ret 4' (0xC2 0x04 0x00) because it's __stdcall
    // x64 uses 'ret' (0xC3)
#ifdef _M_IX86
    unsigned char patch[] = { 0xC2, 0x04, 0x00 };
#elif defined(_M_X64)
    unsigned char patch[] = { 0xC3 };
#endif

    // 3. Change memory protection so we can write to the DLL's code section
    DWORD oldProtect;
    if (VirtualProtect(pAddr, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        // 4. Overwrite the start of the function with our 'ret' instruction
        memcpy(pAddr, patch, sizeof(patch));

        // 5. Restore original memory protection
        VirtualProtect(pAddr, sizeof(patch), oldProtect, &oldProtect);
    }
}
#endif

// Suggested by Gemini: a simple crash handler that installs global handlers for unhandled exceptions and signals in a cross-platform manner.
export class CrashManager 
{
public:
    // Call this once at the very start of main()
    static void init() 
    {
        static CrashManager instance;
    }

private:
    CrashManager() 
    {
        installHandlers();
    }

    void installHandlers() 
    {
#ifdef _WIN32
        // 1. Global filter for all threads (SEH)
        // set_terminate needs be set per-thread, unfortunately, so we rely on SEH for global coverage: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/set-terminate-crt?view=msvc-170.
		SetUnhandledExceptionFilter(windowsExceptionFilter); 

        // 2. Capture abort() calls (triggered by std::terminate)
        signal(SIGABRT, signalHandler);
#else
        // POSIX: Install signal handlers for common fatal signals
        struct sigaction sa;
        sa.sa_handler = signalHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGSEGV, &sa, nullptr); // Segmentation fault
        sigaction(SIGFPE, &sa, nullptr); // Floating point error
        sigaction(SIGABRT, &sa, nullptr); // Abort (incl. std::terminate)
        sigaction(SIGILL, &sa, nullptr); // Illegal instruction
#endif
    }

#ifdef _WIN32
    static LONG WINAPI windowsExceptionFilter(EXCEPTION_POINTERS* info) 
    {
        std::cerr << "[CRASH] Unhandled Exception Code: 0x"
            << std::hex << info->ExceptionRecord->ExceptionCode << std::endl;

        // Here you would typically call MiniDumpWriteDump()

        return EXCEPTION_EXECUTE_HANDLER; // Terminate the process
    }
#endif
    static void signalHandler(int sig)
    {
        std::cerr << "[CRASH] Received signal: " << sig << std::endl;

        // Note: Signal handlers should only call "async-signal-safe" functions.
        // std::cerr and exit() are technically NOT safe, but often used for 
        // last-gasp logging before a forced exit.

        _exit(EXIT_FAILURE);
    }
};
