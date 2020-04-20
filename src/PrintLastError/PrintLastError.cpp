#include <iostream>
#include <Windows.h>

std::wstring GetErrorCodeAsWString(const DWORD errorCode)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
        (LPTSTR)&lpMsgBuf,
        0,
        nullptr);

    std::wstring msg((LPTSTR)lpMsgBuf);
    LocalFree(lpMsgBuf);

    return msg;
}

int main()
{
    std::wcout << GetErrorCodeAsWString(5) << std::endl;

    return 0;
}

