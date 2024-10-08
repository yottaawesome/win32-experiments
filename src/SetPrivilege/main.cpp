// Adapted from https://learn.microsoft.com/en-us/windows/win32/secauthz/enabling-and-disabling-privileges-in-c--
#include <windows.h>
import std;

#pragma comment(lib, "advapi32.lib")

bool SetPrivilege(
    HANDLE hToken,          // access token handle
    std::wstring_view lpszPrivilege,  // name of privilege to enable/disable
    bool bEnablePrivilege   // to enable or disable privilege
)
{
    LUID luid;
    if (not LookupPrivilegeValue(
        nullptr,            // lookup privilege on local system
        lpszPrivilege.data(),   // privilege to lookup 
        &luid))        // receives LUID of privilege
    {
        std::println("LookupPrivilegeValue error: {}", GetLastError());
        return false;
    }

    TOKEN_PRIVILEGES tp {
        .PrivilegeCount = 1,
        .Privileges = {
            {
                .Luid = luid, 
                .Attributes = bEnablePrivilege ? static_cast<DWORD>(SE_PRIVILEGE_ENABLED) : 0
            }
        }
    };

    // Enable the privilege or disable all privileges.
    if (not AdjustTokenPrivileges(
        hToken,
        false,
        &tp,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES)nullptr,
        (PDWORD)nullptr)) 
    {
        std::println("AdjustTokenPrivileges error: {}", GetLastError());
        return false;
    }
    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        std::println("The token does not have the specified privilege.");
        return false;
    }

    return true;
}

namespace Privilege
{
    // See the complete list in winnt.h
    // https://learn.microsoft.com/en-us/windows/win32/secauthz/privilege-constants
    constexpr std::wstring_view AssignPrimaryToken = SE_ASSIGNPRIMARYTOKEN_NAME; 
    constexpr std::wstring_view Backup = SE_BACKUP_NAME;
    constexpr std::wstring_view Debug = SE_DEBUG_NAME;
    constexpr std::wstring_view IncreaseQuota = SE_INCREASE_QUOTA_NAME;
    constexpr std::wstring_view Tcb = SE_TCB_NAME;
    constexpr std::wstring_view SystemTime = SE_SYSTEMTIME_NAME;
}

int main()
{
    HANDLE out = nullptr;
    OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &out);
    SetPrivilege(out, Privilege::SystemTime, false);
    return 0;
}

