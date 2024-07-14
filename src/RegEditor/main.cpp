#include "Header.hpp"
import std;
import common;
import registry;

void OldTests()
{
    const auto key = HKEY_CURRENT_USER;
    std::wstring subKey = L"SOFTWARE\\WinRegTest";
    std::wstring valueA = L"ValueA";
    std::wstring valueB = L"ValueB";
    std::wstring valueAData = L"ValueA";
    std::wstring valueBData = L"ValueB";

    std::wcout << WinReg::DoesKeyExist(key, subKey) << std::endl;

    // Create a key
    WinReg::CreateKey(key, subKey);
    // Write to the keys

    WinReg::WriteKey(key, subKey, valueA, valueAData);
    WinReg::WriteKey(key, subKey, valueB, valueBData);

    // Get the key values
    std::wstring value1 = WinReg::RegGetString(key, subKey, valueA);
    std::wstring value2 = WinReg::RegGetString(key, subKey, valueB);

    std::wcout << value1 << std::endl;
    std::wcout << value2 << std::endl;

    WinReg::DeleteTree(key, subKey);
}

void NewTests()
{
    constexpr auto key = Win32::Registry::Keys::HKCU;
    std::wstring subKey = L"SOFTWARE\\WinRegTest";
    std::wstring valueName = L"ValueName";
    std::wstring data = L"test";

    Registry::CreateKey(key, subKey);
    Registry::Set(key, subKey, valueName, data);
    std::wcout << Registry::Get<std::wstring, true>(key, subKey, valueName) << std::endl;
    std::wcout << Registry::Get<std::wstring, false>(key, subKey, valueName).value() << std::endl;
    WinReg::DeleteTree(key, subKey);
}

int main()
{
    //OldTests();
    NewTests();

    return 0;
}
