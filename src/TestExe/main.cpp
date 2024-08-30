#include "../TestExplicitDLL/ExplicitDLL.hpp"

import common;
import testdll;

using FnPtr_t = decltype(&ExplicitDLL::Something);

int main()
try
{
    TestDLL::TestClass test;
    try
    {
        test.BB();
    }
    catch (const std::runtime_error&)
    {
        std::println("Success! {}", test.GetAString());
    }

    Win32::HMODULE dll = Win32::LoadLibraryW(L"TestExplicitDLL.dll");
    if (not dll)
        throw std::runtime_error("Failed to load DLL");
    
    FnPtr_t something = reinterpret_cast<FnPtr_t>(Win32::GetProcAddress(dll, "Something"));
    if (not something)
        throw std::runtime_error("Failed to load function");

    std::println("Hello World!");
}
catch (const std::exception& ex)
{
    std::println("main() failed: {}", ex.what());
    return 1;
}