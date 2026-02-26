// Adapted from some code Gemini generated for a sample of Detours.
#include <windows.h>
#include <detours/detours.h>

import std;

// 1. Define a function pointer to hold the ORIGINAL function address
static LPTOP_LEVEL_EXCEPTION_FILTER(WINAPI* TrueSetUnhandledExceptionFilter)(
    LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
    ) = SetUnhandledExceptionFilter;

// 2. Define your "Detour" (the replacement function)
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI HookedSetUnhandledExceptionFilter(
    LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
) {
    // We ignore the new filter being passed in. 
    // This "bolts down" the handler by refusing to update it.
    std::cout << "[Detours] Blocked an attempt to overwrite the exception filter!" << std::endl;

    // Return NULL or a dummy value as per the API's expected behavior
    return nullptr;
}

void InstallBoltDownHook() {
    // Detours works in a transaction to ensure all-or-nothing success
    DetourTransactionBegin();

    // Enlist the current thread to be updated during the patch
    DetourUpdateThread(GetCurrentThread());

    // Attach our hook function to the original function
    // This replaces the first few bytes of the original with a jump to ours.
    DetourAttach(&(PVOID&)TrueSetUnhandledExceptionFilter, HookedSetUnhandledExceptionFilter);

    // Commit the changes to memory
    DetourTransactionCommit();
}

void RemoveHook() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    // Detach restores the original bytes
    DetourDetach(&(PVOID&)TrueSetUnhandledExceptionFilter, HookedSetUnhandledExceptionFilter);
    DetourTransactionCommit();
}

int main() 
{
    std::cout << "Installing bolt-down hook..." << std::endl;
    SetUnhandledExceptionFilter([](LPEXCEPTION_POINTERS) -> LONG {
        std::cout << "Original exception filter called!" << std::endl;
        return EXCEPTION_CONTINUE_SEARCH; // Let other handlers run (like the default Windows one)
		});
    InstallBoltDownHook();
    // Test: Attempt to set a new exception filter (this should be blocked)
    SetUnhandledExceptionFilter([](LPEXCEPTION_POINTERS) -> LONG {
        std::cout << "This should never be printed, as the filter update is blocked!" << std::endl;
        return EXCEPTION_CONTINUE_SEARCH;
    });
    std::cout << "Triggering an access violation..." << std::endl;
    int* p = nullptr;
    *p = 42; // This will crash, but our original filter should still work
    // Cleanup (unreachable in this example, but good practice)
    RemoveHook();
    return 0;
}