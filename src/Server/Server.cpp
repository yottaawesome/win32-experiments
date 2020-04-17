#include <iostream>
#include <Windows.h>

// https://devblogs.microsoft.com/oldnewthing/20190425-00/?p=102443
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

void Tokens2(HANDLE hToken)
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

void Tokens()
{
    HANDLE hProcessToken{ 0 };
    HANDLE hRestrictedToken{ 0 };

    /* First get a handle to the current process's primary token */
    if (OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY,
        &hProcessToken))
    {

    }


    /* Create a restricted token with all privileges removed */
    if (CreateRestrictedToken(
        hProcessToken,
        DISABLE_MAX_PRIVILEGE,
        0,
        0,
        0,
        0,
        0,
        0,
        &hRestrictedToken))
    {
        std::wcout << L"OK" << std::endl;
        Tokens2(hRestrictedToken);
    }
    else
    {
        std::wcout << L"Failed" << std::endl;
    }

    PROCESS_INFORMATION pi = { 0 };
    STARTUPINFO         si = { 0 };
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
    Tokens();

    return 0;
}
