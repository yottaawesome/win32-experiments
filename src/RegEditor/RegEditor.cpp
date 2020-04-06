// RegEditor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include "Header.hpp"

int main()
{
    std::wstring subKey = L"SOFTWARE\\Sinefa\\Sinefa Endpoint Agent";

    std::wstring uuidSubKeyValueName = L"AgentUUID";
    std::wstring uuidSubKeyValueData = L"TestValue";

    std::wstring serverSubKeyValueName = L"Server";
    std::wstring serverSubKeyValueData = L"http://127.0.0.1:51934";

    std::wstring regKeyValueName = L"RegistrationKey";
    std::wstring regKeyValueData = L"TestValue";

    std::wstring accountValueName = L"AccountName";
    std::wstring accountValueData = L"TestValue";

    std::wstring accountIdValueName = L"AccountID";
    std::wstring accountIdValueData = L"TestValue";

    std::wcout << WinReg::DoesKeyExist(HKEY_LOCAL_MACHINE, subKey) << std::endl;

    // Create a key
    WinReg::CreateKey(HKEY_LOCAL_MACHINE, subKey);
    // Write to the keys

    WinReg::WriteKey(HKEY_LOCAL_MACHINE, subKey, uuidSubKeyValueName, uuidSubKeyValueData);
    WinReg::WriteKey(HKEY_LOCAL_MACHINE, subKey, serverSubKeyValueName, serverSubKeyValueData);
    WinReg::WriteKey(HKEY_LOCAL_MACHINE, subKey, regKeyValueName, regKeyValueData);
    WinReg::WriteKey(HKEY_LOCAL_MACHINE, subKey, accountValueName, accountValueData);
    WinReg::WriteKey(HKEY_LOCAL_MACHINE, subKey, accountIdValueName, accountIdValueData);

    // Get the key values
    std::wstring value1 = WinReg::RegGetString(HKEY_LOCAL_MACHINE, subKey, uuidSubKeyValueName);
    std::wstring value2 = WinReg::RegGetString(HKEY_LOCAL_MACHINE, subKey, serverSubKeyValueName);
    std::wstring value3 = WinReg::RegGetString(HKEY_LOCAL_MACHINE, subKey, regKeyValueName);
    std::wstring value4 = WinReg::RegGetString(HKEY_LOCAL_MACHINE, subKey, accountValueName);
    std::wstring value5 = WinReg::RegGetString(HKEY_LOCAL_MACHINE, subKey, accountIdValueName);
    
    std::wcout << value1 << std::endl;
    std::wcout << value2 << std::endl;
    std::wcout << value3 << std::endl;
    std::wcout << value4 << std::endl;
    std::wcout << value5 << std::endl;

    return 0;
}
