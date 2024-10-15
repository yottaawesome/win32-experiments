// Adapted from https://learn.microsoft.com/en-gb/windows/win32/secauthz/finding-the-owner-of-a-file-object-in-c--

#include <windows.h>
#include <tchar.h>
#include "accctrl.h"
#include "aclapi.h"

import std;

#pragma comment(lib, "advapi32.lib")

int main()
{
    DWORD dwRtnCode = 0;
    PSID pSidOwner = NULL;
    BOOL bRtnBool = TRUE;
    LPTSTR AcctName = NULL;
    LPTSTR DomainName = NULL;
    DWORD dwAcctName = 1, dwDomainName = 1;
    SID_NAME_USE eUse = SidTypeUnknown;
    HANDLE hFile;
    PSECURITY_DESCRIPTOR pSD = NULL;


    // Get the handle of the file object.
    hFile = CreateFile(
        TEXT("myfile.txt"),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    // Check GetLastError for CreateFile error code.
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD dwErrorCode = 0;

        dwErrorCode = GetLastError();
        std::println("CreateFile error = {}", dwErrorCode);
        return -1;
    }



    // Get the owner SID of the file.
    dwRtnCode = GetSecurityInfo(
        hFile,
        SE_FILE_OBJECT,
        OWNER_SECURITY_INFORMATION,
        &pSidOwner,
        NULL,
        NULL,
        NULL,
        &pSD);

    // Check GetLastError for GetSecurityInfo error condition.
    if (dwRtnCode != ERROR_SUCCESS) {
        DWORD dwErrorCode = 0;

        dwErrorCode = GetLastError();
        std::println("GetSecurityInfo error = {}", dwErrorCode);
        return -1;
    }

    // First call to LookupAccountSid to get the buffer sizes.
    bRtnBool = LookupAccountSid(
        NULL,           // local computer
        pSidOwner,
        AcctName,
        (LPDWORD)&dwAcctName,
        DomainName,
        (LPDWORD)&dwDomainName,
        &eUse);

    // Reallocate memory for the buffers.
    AcctName = (LPTSTR)GlobalAlloc(
        GMEM_FIXED,
        dwAcctName * sizeof(wchar_t));

    // Check GetLastError for GlobalAlloc error condition.
    if (AcctName == NULL) {
        DWORD dwErrorCode = 0;

        dwErrorCode = GetLastError();
        std::println("GlobalAlloc error = {}", dwErrorCode);
        return -1;
    }

    DomainName = (LPTSTR)GlobalAlloc(
        GMEM_FIXED,
        dwDomainName * sizeof(wchar_t));

    // Check GetLastError for GlobalAlloc error condition.
    if (DomainName == NULL) {
        DWORD dwErrorCode = 0;

        dwErrorCode = GetLastError();
        std::println("GlobalAlloc error = {}", dwErrorCode);
        return -1;

    }

    // Second call to LookupAccountSid to get the account name.
    bRtnBool = LookupAccountSid(
        NULL,                   // name of local or remote computer
        pSidOwner,              // security identifier
        AcctName,               // account name buffer
        (LPDWORD)&dwAcctName,   // size of account name buffer 
        DomainName,             // domain name
        (LPDWORD)&dwDomainName, // size of domain name buffer
        &eUse);                 // SID type

    // Check GetLastError for LookupAccountSid error condition.
    if (bRtnBool == FALSE) {
        DWORD dwErrorCode = 0;

        dwErrorCode = GetLastError();

        if (dwErrorCode == ERROR_NONE_MAPPED)
            std::println("Account owner not found for specified SID.");
        else
            std::println("Error in LookupAccountSid.");
        return -1;

    }
    else if (bRtnBool == TRUE)

        // Print the account name.
        std::println("Account owner = {}", AcctName);

    return 0;
}