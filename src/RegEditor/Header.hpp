#pragma once
#include <stdexcept>
#include <Windows.h>

namespace WinReg
{
    class RegistryError : public std::runtime_error
    {
        public:
            RegistryError(const char* message, LONG errorCode);
            LONG ErrorCode() const noexcept;

        private:
            LONG m_errorCode;
    };

    std::wstring RegGetString(
        HKEY hKey,
        const std::wstring& subKey,
        const std::wstring& value
    );

    bool RegGetString(
        HKEY hKey,
        const std::wstring& subKey,
        const std::wstring& value,
        std::wstring& out
    );

    DWORD RegGetDword(
        HKEY hKey,
        const std::wstring& subKey,
        const std::wstring& value
    );

    bool RegGetDword(
        HKEY hKey,
        const std::wstring& subKey,
        const std::wstring& value,
        DWORD& data
    );

    bool DoesKeyExist(
        HKEY hKey,
        const std::wstring& subKey
    );

    bool CreateKey(
        HKEY hKey,
        const std::wstring& subKey
    );

    void WriteKey(
        HKEY hKey,
        std::wstring& subKey,
        std::wstring& keyName,
        std::wstring& keyValue
    );
}
