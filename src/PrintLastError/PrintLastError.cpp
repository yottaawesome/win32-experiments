#include <iostream>
#include <string>
#include <stdexcept>
#include <format>
#include <Windows.h>
#include <winhttp.h>
#include <type_traits>

#pragma comment(lib, "Winhttp.lib")

std::wstring GetErrorCodeAsWString(const DWORD errorCode, const std::wstring& moduleName)
{
    // Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    HMODULE moduleHandle = moduleName.empty() ? nullptr : LoadLibraryW(moduleName.c_str());
    DWORD flags =
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS;
    if (moduleHandle)
        flags |= FORMAT_MESSAGE_FROM_HMODULE;

    FormatMessageW(
        flags,
        moduleHandle,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
        (LPTSTR)&lpMsgBuf,
        0,
        nullptr
    );
    if (moduleHandle)
        FreeLibrary(moduleHandle);
    if (lpMsgBuf == nullptr)
        return L"";

    std::wstring msg((LPTSTR)lpMsgBuf);
    LocalFree(lpMsgBuf);

    return msg;
}

std::string GetErrorCodeAsString(const DWORD errorCode, const std::wstring& moduleName)
{
    // Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    HMODULE moduleHandle = moduleName.empty() ? nullptr : LoadLibraryW(moduleName.c_str());
    DWORD flags =
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS;
    if (moduleHandle)
        flags |= FORMAT_MESSAGE_FROM_HMODULE;

    FormatMessageA(
        flags,
        moduleHandle,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
        (LPSTR)&lpMsgBuf,
        0,
        nullptr
    );
    if (moduleHandle)
        FreeLibrary(moduleHandle);
    if (lpMsgBuf == nullptr)
        return "";

    std::string msg((LPSTR)lpMsgBuf);
    LocalFree(lpMsgBuf);

    return msg;
}

class Win32Error : public std::system_error
{
    public:
        virtual ~Win32Error() {}
    
        Win32Error(const DWORD error) : std::system_error(std::error_code(error, std::system_category())) 
        {
            m_message = std::format("win32 error {} ({:#X}) -- {}", error, error, std::system_error::what());
        }

        Win32Error(const DWORD error, const std::string& msg) : std::system_error(std::error_code(error, std::system_category()), msg) 
        {
            m_message = std::format("win32 error {} ({:#X}) -- {}", error, error, std::system_error::what());
        }

    public:
        virtual const char* what() const noexcept override
        {
            return m_message.c_str();
        }

        virtual DWORD Win32Code() const noexcept
        {
            return static_cast<DWORD>(code().value());
        }

    protected:
        std::string m_message;
};

// See https://akrzemi1.wordpress.com/2017/07/12/your-own-error-code/
// and https://akrzemi1.wordpress.com/examples/error_code-example/
class WinHttpCategory : public std::error_category
{
    public:
        virtual ~WinHttpCategory() noexcept;
        virtual const char* name() const noexcept override;
        virtual std::string message(int ev) const override;
};

WinHttpCategory::~WinHttpCategory() noexcept {}

const char* WinHttpCategory::name() const noexcept
{
    return "WinHttp";
}

std::string WinHttpCategory::message(int ev) const
{
    return GetErrorCodeAsString(static_cast<DWORD>(ev), L"winhttp.dll");
}

void LastErrorToSystemError(const DWORD dwErrVal)
{
    try
    {
        std::error_code ec(dwErrVal, std::system_category());
        throw std::system_error(ec, "Exception occurred");
    }
    catch (const std::system_error& ex)
    {
        std::wcout << ex.what() << std::endl;
    }
}

void LastWinHttpErrorToSystemError(const DWORD dwErrVal)
{
    try
    {
        std::error_code ec(dwErrVal, WinHttpCategory());
        throw std::system_error(ec, "Exception occurred");
    }
    catch (const std::exception& ex)
    {
        std::wcout << ex.what() << std::endl;
    }
}

void ThrowWin32Error(const DWORD errorCode)
{
    try
    {
        throw Win32Error(errorCode);
    }
    catch (const std::exception& ex)
    {
        std::wcout << ex.what() << std::endl;
    }
}

int main()
{
    //std::wcout << GetErrorCodeAsWString(5) << std::endl;
    LastErrorToSystemError(ERROR_ACCESS_DENIED);
    LastWinHttpErrorToSystemError(ERROR_WINHTTP_CANNOT_CALL_AFTER_OPEN);
    ThrowWin32Error(ERROR_READ_FAULT);

    return 0;
}

