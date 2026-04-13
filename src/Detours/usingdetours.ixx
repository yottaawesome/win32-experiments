// Adapted from some code Gemini generated for a sample of Detours.
module;

#include <Windows.h>
#include <detours/detours.h>

export module usingdetours;
import std;

export namespace First
{
    // 1. Define a function pointer to hold the ORIGINAL function address
    using TrueSetUnhandledExceptionFilter = auto(*)(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) -> LPTOP_LEVEL_EXCEPTION_FILTER;

    auto TrueSetUnhandledExceptionFilterPtr = TrueSetUnhandledExceptionFilter{ SetUnhandledExceptionFilter };

    // 2. Define your "Detour" (the replacement function)
    auto HookedSetUnhandledExceptionFilter(
        LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
    ) -> LPTOP_LEVEL_EXCEPTION_FILTER
    {
        // We ignore the new filter being passed in. 
        // This "bolts down" the handler by refusing to update it.
        std::cout << "[Detours] Blocked an attempt to overwrite the exception filter!" << std::endl;

        // Return NULL or a dummy value as per the API's expected behavior
        return nullptr;
    }

    void InstallBoltDownHook() 
    {
        // Detours works in a transaction to ensure all-or-nothing success
        DetourTransactionBegin();

        // Enlist the current thread to be updated during the patch
        DetourUpdateThread(GetCurrentThread());

        // Attach our hook function to the original function
        // This replaces the first few bytes of the original with a jump to ours.
        DetourAttach(&(PVOID&)TrueSetUnhandledExceptionFilterPtr, HookedSetUnhandledExceptionFilter);

        // Commit the changes to memory
        DetourTransactionCommit();
    }

    void RemoveHook() 
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        // Detach restores the original bytes
        DetourDetach(&(PVOID&)TrueSetUnhandledExceptionFilterPtr, HookedSetUnhandledExceptionFilter);
        DetourTransactionCommit();
    }

    int Run()
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
}

export namespace Second
{
    // 1. The Target Class we want to hook
    struct TargetClass 
    {
        void DoWork(int value) 
        {
            std::cout << "Original DoWork called with: " << value << std::endl;
        }
    };

    // 2. The Detour Class
    // This class exists only to provide a compatible __thiscall signature.
    struct DetourClass 
    {
        void Detour_DoWork(int value);

        // Static pointer to hold the original function's address
        static void (TargetClass::* Real_DoWork)(int);
    };

    // Initialize the static member
    void (TargetClass::* DetourClass::Real_DoWork)(int) = &TargetClass::DoWork;

    // 3. The Implementation of our Hook
    void DetourClass::Detour_DoWork(int value) 
    {
        std::cout << "[Hook] Intercepted DoWork! Changing value from " << value << " to " << (value * 2) << std::endl;

        // Call the original function using the trampoline.
        // 'this' is actually a TargetClass* at runtime because Detours
        // redirected the call, so we reinterpret_cast accordingly.
        (reinterpret_cast<TargetClass*>(this)->*Real_DoWork)(value * 2);
    }

    void InstallMemberHook() 
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        // Detours requires a raw pointer (PVOID) for the target.
        // C++ member pointers are tricky to cast, so we use a union or reference cast.
        // Real_DoWork is a static variable, so we can take its address directly.
        PVOID pRawTarget = *(PVOID*)&DetourClass::Real_DoWork;
        // Detour_DoWork is a member-function-pointer literal; store it in a
        // local variable first so we have an addressable lvalue to cast from.
        auto detourFn = &DetourClass::Detour_DoWork;
        PVOID pRawDetour = *(PVOID*)&detourFn;

        DetourAttach(&pRawTarget, pRawDetour);

        if (DetourTransactionCommit() == NO_ERROR) 
        {
            // After commit, the original function pointer is updated to point to the trampoline
            *(PVOID*)&DetourClass::Real_DoWork = pRawTarget;
            std::cout << "Member hook installed successfully." << std::endl;
        }
    }

    int Run()
    {
        TargetClass myObj;

        std::cout << "--- Before Hook ---" << std::endl;
        myObj.DoWork(10);

        InstallMemberHook();

        std::cout << "\n--- After Hook ---" << std::endl;
        myObj.DoWork(10);
        return 0;
    }
}

export namespace Third
{
    // 1. The Class with Virtual Functions
    class Base {
    public:
        virtual void Speak() { std::cout << "Original: Hello!" << std::endl; }
    };

    // 2. The Detour Class (to provide __thiscall signature)
    class DetourClass {
    public:
        void Detour_Speak();

        // Function pointer to hold the trampoline
        static void (Base::* Real_Speak)();
    };

    // Initialize trampoline to null
    void (Base::* DetourClass::Real_Speak)() = nullptr;

    void DetourClass::Detour_Speak() {
        std::cout << "[Hook] Intercepted Speak! Silence!" << std::endl;
        // Call original via trampoline
        (reinterpret_cast<Base*>(this)->*Real_Speak)();
    }

    void InstallVirtualHook() {
        // A. Create a dummy instance to find the vtable
        Base* pDummy = new Base();

        // B. Get the vtable pointer (offset 0)
        // On MSVC, the first member of the object is the vtable pointer.
        void*** vptr = reinterpret_cast<void***>(pDummy);
        void** vtable = *vptr;

        // C. Get the function address from the correct slot.
        // Slot 0 is usually the first virtual function (Speak)
        void* pTargetFunc = vtable[0];

        // D. Standard Detours transaction
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        // We store the target address into our trampoline pointer
        *(void**)&DetourClass::Real_Speak = pTargetFunc;

        // Attach our detour function
        auto detourFn = &DetourClass::Detour_Speak;
        PVOID pRawDetour = *(PVOID*)&detourFn;
        DetourAttach(&(PVOID&)DetourClass::Real_Speak, pRawDetour);

        DetourTransactionCommit();

        delete pDummy;
    }

    int Run() {
        Base* myObj = new Base();

        std::cout << "--- Before Hook ---" << std::endl;
        myObj->Speak();

        InstallVirtualHook();

        std::cout << "\n--- After Hook ---" << std::endl;
        myObj->Speak(); // This call now goes through our hook

        return 0;
    }
}
