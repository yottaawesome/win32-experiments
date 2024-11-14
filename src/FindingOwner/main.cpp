// Adapted from https://learn.microsoft.com/en-gb/windows/win32/secauthz/finding-the-owner-of-a-file-object-in-c--

#include <windows.h>
#include <tchar.h>
#include "accctrl.h"
#include "aclapi.h"

import std;

#pragma comment(lib, "advapi32.lib")

int main()
{
    // Get the handle of the file object.
    HANDLE hFile = CreateFile(
        TEXT("myfile.txt"),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    // Check GetLastError for CreateFile error code.
    if (hFile == INVALID_HANDLE_VALUE) 
    {
        DWORD dwErrorCode = GetLastError();
        std::println("CreateFile error = {}", dwErrorCode);
        return -1;
    }

    // Get the owner SID of the file.
    PSID pSidOwner = nullptr;
    PSECURITY_DESCRIPTOR pSD = nullptr;
    DWORD dwRtnCode = GetSecurityInfo(
        hFile,
        SE_FILE_OBJECT,
        OWNER_SECURITY_INFORMATION,
        &pSidOwner,
        nullptr,
        nullptr,
        nullptr,
        &pSD);

    // Check GetLastError for GetSecurityInfo error condition.
    if (dwRtnCode != ERROR_SUCCESS) 
    {
        DWORD dwErrorCode = GetLastError();
        std::println("GetSecurityInfo error = {}", dwErrorCode);
        return -1;
    }

    // First call to LookupAccountSid to get the buffer sizes.
    LPWSTR AcctName = nullptr;
    LPWSTR DomainName = nullptr;
    DWORD dwAcctName = 1;
    DWORD dwDomainName = 1;
    SID_NAME_USE eUse = SidTypeUnknown;
    bool bRtnBool = LookupAccountSid(
        nullptr,           // local computer
        pSidOwner,
        AcctName,
        (LPDWORD)&dwAcctName,
        DomainName,
        (LPDWORD)&dwDomainName,
        &eUse);

    // Reallocate memory for the buffers.
    AcctName = (LPWSTR)GlobalAlloc(GMEM_FIXED, dwAcctName * sizeof(wchar_t));

    // Check GetLastError for GlobalAlloc error condition.
    if (AcctName == nullptr) 
    {
        DWORD dwErrorCode = GetLastError();
        std::println("GlobalAlloc error = {}", dwErrorCode);
        return -1;
    }

    DomainName = (LPTSTR)GlobalAlloc(GMEM_FIXED, dwDomainName * sizeof(wchar_t));

    // Check GetLastError for GlobalAlloc error condition.
    if (DomainName == nullptr)
    {
        DWORD dwErrorCode = GetLastError();
        std::println("GlobalAlloc error = {}", dwErrorCode);
        return -1;
    }

    // Second call to LookupAccountSid to get the account name.
    bRtnBool = LookupAccountSid(
        nullptr,                   // name of local or remote computer
        pSidOwner,              // security identifier
        AcctName,               // account name buffer
        (LPDWORD)&dwAcctName,   // size of account name buffer 
        DomainName,             // domain name
        (LPDWORD)&dwDomainName, // size of domain name buffer
        &eUse);                 // SID type

    // Check GetLastError for LookupAccountSid error condition.
    if (not bRtnBool) 
    {
        DWORD dwErrorCode = GetLastError();
        if (dwErrorCode == ERROR_NONE_MAPPED)
            std::println("Account owner not found for specified SID.");
        else
            std::println("Error in LookupAccountSid.");
        return -1;
    }

    // Print the account name.
    std::wcout << std::format(L"Account owner = {}\n", AcctName);

    return 0;
}
