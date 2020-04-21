#include <iostream>
#include <Windows.h>

#define MAX_NAME 256

void RemoveSid()
{
    BYTE sidBuffer[256];
    PSID pAdminSID = (PSID)sidBuffer;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    if(!AllocateAndInitializeSid(&SIDAuth, 2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
        &pAdminSID)) 
    {
        
    }
    // Change the local administrator’s SID to a deny-only SID.
    SID_AND_ATTRIBUTES SidToDisable[1];
    SidToDisable[0].Sid = pAdminSID;
    SidToDisable[0].Attributes = 0;
}

BOOL SearchTokenGroupsForSID(HANDLE hToken)
{
    DWORD i, dwSize = 0, dwResult = 0;
    PTOKEN_GROUPS pGroupInfo;
    SID_NAME_USE SidType;
    PSID pSID = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;

    // Open a handle to the access token for the calling process.

    /*if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        printf("OpenProcessToken Error %u\n", GetLastError());
        return FALSE;
    }*/

    // Call GetTokenInformation to get the buffer size.

    if (!GetTokenInformation(hToken, TokenGroups, NULL, dwSize, &dwSize))
    {
        dwResult = GetLastError();
        if (dwResult != ERROR_INSUFFICIENT_BUFFER) {
            printf("GetTokenInformation Error %u\n", dwResult);
            return FALSE;
        }
    }

    // Allocate the buffer.

    pGroupInfo = (PTOKEN_GROUPS)GlobalAlloc(GPTR, dwSize);

    // Call GetTokenInformation again to get the group information.

    if (!GetTokenInformation(hToken, TokenGroups, pGroupInfo,
        dwSize, &dwSize))
    {
        printf("GetTokenInformation Error %u\n", GetLastError());
        return FALSE;
    }

    // Create a SID for the BUILTIN\Administrators group.

    if (!AllocateAndInitializeSid(
        &SIDAuth,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pSID))
    {
        printf("AllocateAndInitializeSid Error %u\n", GetLastError());
        return FALSE;
    }

    // Loop through the group SIDs looking for the administrator SID.

    for (i = 0; i < pGroupInfo->GroupCount; i++)
    {
        if (EqualSid(pSID, pGroupInfo->Groups[i].Sid))
        {
            DWORD dwSize = 250;
            std::wstring lpName;
            std::wstring lpDomain;
            lpName.resize(dwSize);
            lpDomain.resize(dwSize);
            // Lookup the account m_name and print it.

            dwSize = MAX_NAME;
            if (!LookupAccountSid(NULL, pGroupInfo->Groups[i].Sid,
                &lpName[0], &dwSize, &lpDomain[0],
                &dwSize, &SidType))
            {
                //dwResult = GetLastError();
                //if (dwResult == ERROR_NONE_MAPPED)
                //    strcpy_s(lpName, dwSize, "NONE_MAPPED");
                //else
               // {
                //    printf("LookupAccountSid Error %u\n", GetLastError());
                //    return FALSE;
               // }
            }

            lpName.shrink_to_fit();
            lpDomain.shrink_to_fit();
            std::wcout << L"Current user is a member of the " << lpDomain << "\\" << lpName << std::endl;

            // Find out whether the SID is enabled in the token.
            if (pGroupInfo->Groups[i].Attributes & SE_GROUP_ENABLED)
                printf("The group SID is enabled.\n");
            else if (pGroupInfo->Groups[i].Attributes &
                SE_GROUP_USE_FOR_DENY_ONLY)
                printf("The group SID is a deny-only SID.\n");
            else
                printf("The group SID is not enabled.\n");
        }
    }

    if (pSID)
        FreeSid(pSID);
    if (pGroupInfo)
        GlobalFree(pGroupInfo);
    return TRUE;
}

//https://docs.microsoft.com/en-us/windows/win32/secauthz/enabling-and-disabling-privileges-in-c--
//https://docs.microsoft.com/en-us/windows/win32/secauthz/defining-permissions-in-c--
//https://docs.microsoft.com/en-us/windows/win32/wmisdk/executing-privileged-operations-using-c-
// https://devblogs.microsoft.com/oldnewthing/20190425-00/?p=102443
//https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessasuserw
//https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-createrestrictedtoken
//https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createprocesswithlogonw
//https://stackoverflow.com/questions/5629383/createprocess-running-as-user-nt-authority-network-service-without-knowing-th
//https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-logonuserw
//https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-logonuserexw
//https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-impersonateloggedonuser
//https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-sid_and_attributes
//https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-gettokeninformation
//https://docs.microsoft.com/en-us/windows/win32/api/winnt/ne-winnt-token_information_class
//https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-token_privileges
//https://docs.microsoft.com/en-us/windows/win32/secauthz/privileges
// https://docs.microsoft.com/en-us/windows/win32/secauthz/privilege-constants
//https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocesstoken
//https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getshellwindow
//https://docs.microsoft.com/en-us/windows/win32/secbp/changing-privileges-in-a-token
//https://docs.microsoft.com/en-us/windows/win32/secbp/creating-a-dacl
//https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupprivilegenamew
//https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getcurrentprocesstoken
//https://www.oreilly.com/library/view/secure-programming-cookbook/0596003943/ch01s02.html
//https://docs.microsoft.com/en-us/windows/win32/secauthz/verifying-client-access-with-acls-in-c--
//https://stackoverflow.com/questions/30970433/do-high-integrity-tokens-have-to-have-the-administrators-group-enabled
//https://docs.microsoft.com/en-us/windows/win32/secauthz/well-known-sids
//https://docs.microsoft.com/en-us/windows/win32/secauthz/searching-for-a-sid-in-an-access-token-in-c--
//https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-lookupaccountsida

/*int test(int, char**)
{
    HWND hwnd = GetShellWindow();

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    HANDLE process =
        OpenProcess(PROCESS_CREATE_PROCESS, FALSE, pid);

    SIZE_T size;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
    auto p = (PPROC_THREAD_ATTRIBUTE_LIST)new char[size];

    InitializeProcThreadAttributeList(p, 1, 0, &size);
    UpdateProcThreadAttribute(p, 0,
        PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,
        &process, sizeof(process),
        nullptr, nullptr);

    wchar_t cmd[] = L"C:\\Windows\\System32\\cmd.exe";
    STARTUPINFOEX siex = {};
    siex.lpAttributeList = p;
    siex.StartupInfo.cb = sizeof(siex);
    PROCESS_INFORMATION pi;

    CreateProcessW(cmd, cmd, nullptr, nullptr, FALSE,
        CREATE_NEW_CONSOLE | EXTENDED_STARTUPINFO_PRESENT,
        nullptr, nullptr, &siex.StartupInfo, &pi);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    delete[](char*)p;
    CloseHandle(process);
    return 0;
}*/

void Logon()
{
    HANDLE hToken;

    //SYSTEM:NT AUTHORITY -> LocalSystem :: Only services can run with this user
    //LocalService:NT AUTHORITY -> LocalService :: https://docs.microsoft.com/en-us/windows/win32/services/localservice-account. Can only be logged on via a process running in the contect of SYSTEM.
    //NetworkService:NT AUTHORITY -> NetworkService :: https://docs.microsoft.com/en-us/windows/win32/services/networkservice-account. Can only be logged on via a process running in the contect of SYSTEM.
    //https://microsoft.public.platformsdk.security.narkive.com/rX9L5iLB/logonuser-and-network-service-failure
    if (LogonUser(L"SYSTEM", L"NT AUTHORITY", NULL, LOGON32_LOGON_SERVICE, LOGON32_PROVIDER_DEFAULT, &hToken))
    {
        std::wcout << L"Succeeded" << std::endl;
    }
    else
    {
        std::wcout << L"Failed " << GetLastError() << std::endl;
    }

    CloseHandle(hToken);
}

void Logon2()
{
    PROCESS_INFORMATION pi = { 0 };
    STARTUPINFO         si = { 0 };

    if (CreateProcessWithLogonW(
        L"NetworkService",
        L"NT AUTHORITY",
        nullptr,
        LOGON_WITH_PROFILE,
        L"A:\\Code\\C++\\win32-experiments\\src\\x64\\Debug\\Client.exe",
        nullptr,
        CREATE_UNICODE_ENVIRONMENT,
        NULL,
        NULL,
        &si,
        &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else
    {
        std::wcout << L"Failed " << GetLastError() << std::endl;
    }
}

void EnumerateTokenPrivileges(HANDLE hToken)
{
    DWORD dwLen;
    bool bRes;

    // obtain dwLen
    bRes = GetTokenInformation(
        hToken,
        TokenPrivileges,
        NULL,
        0,
        &dwLen
    );

    BYTE* pBuffer = new BYTE[dwLen];

    bRes = GetTokenInformation(
        hToken,
        TokenPrivileges,
        pBuffer,
        dwLen,
        &dwLen
    );

    TOKEN_PRIVILEGES* pPrivs = (TOKEN_PRIVILEGES*)pBuffer;
    for (DWORD i = 0; i < pPrivs->PrivilegeCount; i++)
    {
        DWORD size = 200;
        std::wstring buffer;
        buffer.resize(size);
        if (LookupPrivilegeName(nullptr, &pPrivs->Privileges[i].Luid, &buffer[0], &size))
        {
            buffer.shrink_to_fit();
            std::wcout << buffer << std::endl;
        }
        else
        {
            std::wcout << L"Failed to lookup privileged" << std::endl;
        }
    }
}

void LaunchRestrictedProcess()
{
    HANDLE hProcessToken{ 0 };
    HANDLE hRestrictedToken{ 0 };

    /* First get a handle to the current process's primary token */
    if (!OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY,
        &hProcessToken))
    {
        throw std::runtime_error("Failed OpenProcessToken()");
    }

    BYTE sidBuffer[256];
    PSID pAdminSID = (PSID)sidBuffer;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(
        &SIDAuth,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 
        0, 
        0, 
        0, 
        0, 
        0, 
        0,
        &pAdminSID))
    {
        throw std::runtime_error("Failed to initialise SID");
    }
    // Change the local administrator’s SID to a deny-only SID.
    SID_AND_ATTRIBUTES SidToDisable[1];
    SidToDisable[0].Sid = pAdminSID;
    SidToDisable[0].Attributes = 0;

    /* Create a restricted token with all privileges removed */
    if (CreateRestrictedToken(
        hProcessToken,
        DISABLE_MAX_PRIVILEGE,
        1,
        SidToDisable,
        0,
        0,
        0,
        0,
        &hRestrictedToken))
    {
        std::wcout << L"OK" << std::endl;
        EnumerateTokenPrivileges(hRestrictedToken);
    }
    else
    {
        throw std::runtime_error("Failed CreateRestrictedToken()");
    }

    if (pAdminSID)
        FreeSid(pAdminSID);

    SearchTokenGroupsForSID(hRestrictedToken);

    PROCESS_INFORMATION pi = { 0 };
    STARTUPINFO         si = { 0 };
    si.cb = sizeof(si);
    wchar_t commandLine[] = L"Client.exe lala";
    /* Create a new process using the restricted token */
    if (CreateProcessAsUser(
        hRestrictedToken,
        L"A:\\Code\\C++\\win32-experiments\\src\\x64\\Debug\\Client.exe",
        commandLine,
        nullptr,
        nullptr,
        true,
        CREATE_UNICODE_ENVIRONMENT,
        nullptr,
        nullptr,
        &si,
        &pi
    ))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        std::wcout << "OK" << std::endl;
    }

    /* Cleanup */
    CloseHandle(hRestrictedToken);
    CloseHandle(hProcessToken);
}

int main(int argc, char** args)
{
    LaunchRestrictedProcess();

    return 0;
}
