#include <iostream>
#include <string>
#include <stdexcept>
#include <format>
#include <Windows.h>
#include <winhttp.h>
#include <type_traits>

#pragma comment(lib, "Winhttp.lib")

template<class T, class S>
using is_string = std::is_same<T, typename S::value_type>;

template<typename STR_T, typename STR_V = STR_T::value_type>
STR_T TranslateErrorCode(const DWORD errorCode, const std::wstring& moduleName)
{
    static_assert(std::is_same<std::basic_string<char>, STR_T>::value || std::is_same<std::basic_string<wchar_t>, STR_T>::value, __FUNCTION__ "(): STR_T must be either a std::string or std::wstring");

    // Retrieve the system error message for the last-error code
    void* messageBuffer = nullptr;
    HMODULE moduleHandle = moduleName.empty() ? nullptr : LoadLibraryW(moduleName.c_str());
    const DWORD flags =
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        (moduleHandle ? FORMAT_MESSAGE_FROM_HMODULE : 0);

    if (std::is_same<STR_V, char>::value)
    {
        FormatMessageA(
            flags,
            moduleHandle,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
            (char*)&messageBuffer,
            0,
            nullptr
        );
    }
    else
    {
        FormatMessageW(
            flags,
            moduleHandle,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
            (wchar_t*)&messageBuffer,
            0,
            nullptr
        );
    }
    
    if (moduleHandle)
        FreeLibrary(moduleHandle);
    if (messageBuffer == nullptr)
        return STR_T();

    STR_T msg((STR_V*)messageBuffer);
    LocalFree(messageBuffer);

    return msg;
}

template<typename T>
class ErrorUtil
{
    typedef T(*PTranslateErrorCode)(const DWORD, const std::wstring&);

    public:
        static PTranslateErrorCode TranslateErrorCode;
};

template<typename T>
ErrorUtil<T>::PTranslateErrorCode ErrorUtil<T>::TranslateErrorCode = ::TranslateErrorCode<T>;

using ErrorUtilW = ErrorUtil<std::wstring>;




std::wstring GetErrorCodeAsWString(const DWORD errorCode, const std::wstring& moduleName)
{
    // Retrieve the system error message for the last-error code
    void* messageBuffer = nullptr;
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
        (wchar_t*)&messageBuffer,
        0,
        nullptr
    );
    if (moduleHandle)
        FreeLibrary(moduleHandle);
    if (messageBuffer == nullptr)
        return L"";

    std::wstring msg((LPWSTR)messageBuffer); const std::wstring::value_type* x = L"A";
    LocalFree(messageBuffer);

    return msg;
}

std::string GetErrorCodeAsString(const DWORD errorCode, const std::wstring& moduleName)
{
    // Retrieve the system error message for the last-error code
    void* messageBuffer = nullptr;
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
        (char*)&messageBuffer,
        0,
        nullptr
    );
    if (moduleHandle)
        FreeLibrary(moduleHandle);
    if (messageBuffer == nullptr)
        return "";

    std::string msg((LPSTR)messageBuffer);
    LocalFree(messageBuffer);

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
    //std::wstring s = TranslateErrorCode<std::wstring>(5, L"");
    ErrorUtilW::TranslateErrorCode(5, L"");

    //std::wcout << GetErrorCodeAsWString(5) << std::endl;
    LastErrorToSystemError(ERROR_ACCESS_DENIED);
    LastWinHttpErrorToSystemError(ERROR_WINHTTP_CANNOT_CALL_AFTER_OPEN);
    ThrowWin32Error(ERROR_READ_FAULT);

    return 0;
}

