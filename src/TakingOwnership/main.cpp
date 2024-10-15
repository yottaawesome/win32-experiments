// Adapted from https://learn.microsoft.com/en-us/windows/win32/secauthz/taking-object-ownership-in-c--

#include <windows.h>
#include <stdio.h>
#include <accctrl.h>
#include <aclapi.h>

import std;

//Forward declaration of SetPrivilege
BOOL SetPrivilege(
    HANDLE hToken,          // access token handle
    LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
    BOOL bEnablePrivilege   // to enable or disable privilege
)
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(
        NULL,            // lookup privilege on local system
        lpszPrivilege,   // privilege to lookup 
        &luid))        // receives LUID of privilege
    {
        std::println("LookupPrivilegeValue error: {}", GetLastError());
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // Enable the privilege or disable all privileges.

    if (!AdjustTokenPrivileges(
        hToken,
        FALSE,
        &tp,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES)NULL,
        (PDWORD)NULL))
    {
        std::println("AdjustTokenPrivileges error: {}", GetLastError());
        return FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

    {
        std::println("The token does not have the specified privilege.");
        return FALSE;
    }

    return TRUE;
}


BOOL TakeOwnership(LPTSTR lpszOwnFile)
{

    BOOL bRetval = FALSE;

    HANDLE hToken = NULL;
    PSID pSIDAdmin = NULL;
    PSID pSIDEveryone = NULL;
    PACL pACL = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld =
        SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    const int NUM_ACES = 2;
    EXPLICIT_ACCESS ea[NUM_ACES];
    DWORD dwRes;

    // Specify the DACL to use.
    // Create a SID for the Everyone group.
    if (!AllocateAndInitializeSid(&SIDAuthWorld, 1,
        SECURITY_WORLD_RID,
        0,
        0, 0, 0, 0, 0, 0,
        &pSIDEveryone))
    {
        std::println("AllocateAndInitializeSid (Everyone) error {}",
            GetLastError());
        goto Cleanup;
    }

    // Create a SID for the BUILTIN\Administrators group.
    if (!AllocateAndInitializeSid(&SIDAuthNT, 2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pSIDAdmin))
    {
        std::println("AllocateAndInitializeSid (Admin) error {}",
            GetLastError());
        goto Cleanup;
    }

    ZeroMemory(&ea, NUM_ACES * sizeof(EXPLICIT_ACCESS));

    // Set read access for Everyone.
    ea[0].grfAccessPermissions = GENERIC_READ;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance = NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName = (LPTSTR)pSIDEveryone;

    // Set full control for Administrators.
    ea[1].grfAccessPermissions = GENERIC_ALL;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance = NO_INHERITANCE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[1].Trustee.ptstrName = (LPTSTR)pSIDAdmin;

    if (ERROR_SUCCESS != SetEntriesInAcl(NUM_ACES,
        ea,
        NULL,
        &pACL))
    {
        std::println("Failed SetEntriesInAcl");
        goto Cleanup;
    }

    // Try to modify the object's DACL.
    dwRes = SetNamedSecurityInfo(
        lpszOwnFile,                 // name of the object
        SE_FILE_OBJECT,              // type of object
        DACL_SECURITY_INFORMATION,   // change only the object's DACL
        NULL, NULL,                  // do not change owner or group
        pACL,                        // DACL specified
        NULL);                       // do not change SACL

    if (ERROR_SUCCESS == dwRes)
    {
        std::println("Successfully changed DACL");
        bRetval = TRUE;
        // No more processing needed.
        goto Cleanup;
    }
    if (dwRes != ERROR_ACCESS_DENIED)
    {
        std::println("First SetNamedSecurityInfo call failed: {}",
            dwRes);
        goto Cleanup;
    }

    // If the preceding call failed because access was denied, 
    // enable the SE_TAKE_OWNERSHIP_NAME privilege, create a SID for 
    // the Administrators group, take ownership of the object, and 
    // disable the privilege. Then try again to set the object's DACL.

    // Open a handle to the access token for the calling process.
    if (!OpenProcessToken(GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES,
        &hToken))
    {
        std::println("OpenProcessToken failed: {}", GetLastError());
        goto Cleanup;
    }

    // Enable the SE_TAKE_OWNERSHIP_NAME privilege.
    if (!SetPrivilege(hToken, SE_TAKE_OWNERSHIP_NAME, TRUE))
    {
        std::println("You must be logged on as Administrator.");
        goto Cleanup;
    }

    // Set the owner in the object's security descriptor.
    dwRes = SetNamedSecurityInfo(
        lpszOwnFile,                 // name of the object
        SE_FILE_OBJECT,              // type of object
        OWNER_SECURITY_INFORMATION,  // change only the object's owner
        pSIDAdmin,                   // SID of Administrator group
        NULL,
        NULL,
        NULL);

    if (dwRes != ERROR_SUCCESS)
    {
        std::println("Could not set owner. Error: {}", dwRes);
        goto Cleanup;
    }

    // Disable the SE_TAKE_OWNERSHIP_NAME privilege.
    if (!SetPrivilege(hToken, SE_TAKE_OWNERSHIP_NAME, FALSE))
    {
        std::println("Failed SetPrivilege call unexpectedly.");
        goto Cleanup;
    }

    // Try again to modify the object's DACL,
    // now that we are the owner.
    dwRes = SetNamedSecurityInfo(
        lpszOwnFile,                 // name of the object
        SE_FILE_OBJECT,              // type of object
        DACL_SECURITY_INFORMATION,   // change only the object's DACL
        NULL, NULL,                  // do not change owner or group
        pACL,                        // DACL specified
        NULL);                       // do not change SACL

    if (dwRes == ERROR_SUCCESS)
    {
        std::println("Successfully changed DACL");
        bRetval = TRUE;
    }
    else
    {
        std::println("Second SetNamedSecurityInfo call failed: {}",
            dwRes);
    }

Cleanup:

    if (pSIDAdmin)
        FreeSid(pSIDAdmin);

    if (pSIDEveryone)
        FreeSid(pSIDEveryone);

    if (pACL)
        LocalFree(pACL);

    if (hToken)
        CloseHandle(hToken);

    return bRetval;

}

int main()
{
    return 0;
}