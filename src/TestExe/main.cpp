#include "../TestExplicitDLL/ExplicitDLL.hpp"

import common;
import testdll;

using FnPtr_t = decltype(&ExplicitDLL::Something);
using FnMakePtr_t = decltype(&ExplicitDLL::Make);

int main()
try
{
    TestDLL::TestClass test;
    TestDLL::AnotherFunction();
    TestDLL::ReturnIt(1);
    TestDLL::ReturnIt(1.f);
    TestDLL::YetAnotherFunction();
    try
    {
        test.BB();
    }
    catch (const std::runtime_error&)
    {
        std::println("Success! {}", test.GetAString());
    }
    TestDLL::TestClass2 test2;
    test2.CC();

    Win32::HMODULE dll = Win32::LoadLibraryW(L"TestExplicitDLL.dll");
    if (not dll)
        throw std::runtime_error("Failed to load DLL");
    
    FnPtr_t something = reinterpret_cast<FnPtr_t>(Win32::GetProcAddress(dll, "Something"));
    if (not something)
        throw std::runtime_error("Failed to load function");

    FnMakePtr_t maker = reinterpret_cast<FnMakePtr_t>(Win32::GetProcAddress(dll, "Make"));
    if (not maker)
        throw std::runtime_error("Failed to load function");

    auto made = maker();
    made->DoIt();

    std::println("Hello World!");
}
catch (const std::exception& ex)
{
    std::println("main() failed: {}", ex.what());
    return 1;
}