#include <iostream>
#include <Windows.h>

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

int wmain(int argc, wchar_t** args)
{
    for(int i = 0; i < argc; i++)
        std::wcout << args[i] << std::endl;

    HANDLE hProcessToken{ 0 };
    /* First get a handle to the current process's primary token */
    OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY,
        &hProcessToken);
    EnumerateTokenPrivileges(hProcessToken);

    std::wcout << "Hello World!" << std::endl;

    HANDLE hFile = CreateFile(
        L"C:\\test.txt",                // name of the write
        GENERIC_WRITE,          // open for writing
        0,                      // do not share
        NULL,                   // default security
        OPEN_ALWAYS,             // create new file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        std::wcout << "Failed to create file handle! " << GetLastError() << std::endl;
    }
    else
    {
        std::wcout << "Should not have succeeded!" << std::endl;
    }

    return 0;
}
