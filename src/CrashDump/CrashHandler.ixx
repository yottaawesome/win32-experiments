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
