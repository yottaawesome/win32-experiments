#include "Header.hpp"

namespace WinReg
{
    // Adapted from https://docs.microsoft.com/en-us/archive/msdn-magazine/2017/may/c-use-modern-c-to-access-the-windows-registry
    std::wstring RegGetString(
        HKEY hKey,
        const std::wstring& subKey,
        const std::wstring& value
    )
    {
        DWORD dataSize{};
        LONG retCode = RegGetValue(
            hKey,
            subKey.c_str(),
            value.c_str(),
            RRF_RT_REG_SZ,
            nullptr,
            nullptr,
            &dataSize
        );

        if (retCode != ERROR_SUCCESS)
            throw RegistryError{ "Cannot read string from registry", retCode };

        std::wstring data;
        data.resize(dataSize / sizeof(wchar_t));

        retCode = RegGetValue(
            hKey,
            subKey.c_str(),
            value.c_str(),
            RRF_RT_REG_SZ,
            nullptr,
            &data[0],
            &dataSize
        );

        if (retCode != ERROR_SUCCESS)
            throw RegistryError{ "Cannot read string from registry", retCode };

        DWORD stringLengthInWchars = dataSize / sizeof(wchar_t);

        stringLengthInWchars--; // Exclude the NUL written by the Win32 API
        data.resize(stringLengthInWchars);

        return data;
    }

    bool RegGetString(
        HKEY hKey,
        const std::wstring& subKey,
        const std::wstring& value,
        std::wstring& out
    )
    {
        DWORD dataSize{};
        LONG retCode = RegGetValue(
            hKey,
            subKey.c_str(),
            value.c_str(),
            RRF_RT_REG_SZ,
            nullptr,
            nullptr,
            &dataSize
        );

        if (retCode != ERROR_SUCCESS)
            return false;

        std::wstring data;
        data.resize(dataSize / sizeof(wchar_t));

        retCode = RegGetValue(
            hKey,
            subKey.c_str(),
            value.c_str(),
            RRF_RT_REG_SZ,
            nullptr,
            &data[0],
            &dataSize
        );

        if (retCode != ERROR_SUCCESS)
            return false;

        DWORD stringLengthInWchars = dataSize / sizeof(wchar_t);

        stringLengthInWchars--; // Exclude the NUL written by the Win32 API
        data.resize(stringLengthInWchars);

        out = data;

        return true;
    }

    DWORD RegGetDword(
        HKEY hKey,
        const std::wstring& subKey,
        const std::wstring& value
    )
    {
        DWORD data{};
        DWORD dataSize = sizeof(data);
        LONG retCode = ::RegGetValue(
            hKey,
            subKey.c_str(),
            value.c_str(),
            RRF_RT_REG_DWORD,
            nullptr,
            &data,
            &dataSize
        );
        if (retCode != ERROR_SUCCESS)
        {
            throw RegistryError{ "Cannot read DWORD from registry.", retCode };
        }
        return data;
    }

    bool RegGetDword(
        HKEY hKey,
        const std::wstring& subKey,
        const std::wstring& value,
        DWORD& data
    )
    {
        DWORD dataSize = sizeof(data);
        LONG retCode = ::RegGetValue(
            hKey,
            subKey.c_str(),
            value.c_str(),
            RRF_RT_REG_DWORD,
            nullptr,
            &data,
            &dataSize
        );
        return retCode != ERROR_SUCCESS;
    }

    bool DoesKeyExist(
        HKEY hKey,
        const std::wstring& subKey
    )
    {
        HKEY openedKey = nullptr;
        LSTATUS result = RegOpenKeyEx(hKey, subKey.c_str(), 0, KEY_READ, &openedKey);

        if (openedKey)
            RegCloseKey(openedKey);

        return result == ERROR_SUCCESS;
    }

    bool CreateKey(
        HKEY hKey,
        const std::wstring& subKey
    )
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regcreatekeyexw
        DWORD disposition;
        HKEY openedKey;
        DWORD createKeyStatus =
            RegCreateKeyEx(
                hKey,
                subKey.c_str(),
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                NULL,
                &openedKey,
                &disposition);

        if (createKeyStatus != ERROR_SUCCESS)
            throw RegistryError{ "Failed to write to registry", (LONG)createKeyStatus };
        RegCloseKey(openedKey);

        return disposition == REG_CREATED_NEW_KEY;
    }

    void WriteKey(
        HKEY hKey,
        const std::wstring& subKey,
        const std::wstring& keyName,
        const std::wstring& keyValue
    )
    {
        HKEY openedKey = nullptr;
        DWORD openKeyStatus = RegOpenKeyEx(
            hKey,
            subKey.c_str(),
            0,
            KEY_WRITE,
            &openedKey
        );

        if (openKeyStatus != ERROR_SUCCESS)
            throw RegistryError{ "Failed to open registry key", (LONG)openKeyStatus };

        DWORD setValueStatus =
            RegSetValueEx(
                openedKey,
                keyName.c_str(),
                0,
                REG_SZ,
                (LPBYTE)(keyValue.c_str()),
                (keyValue.size() + 1) * sizeof(wchar_t)
            );

        RegCloseKey(openedKey);

        if (setValueStatus != ERROR_SUCCESS)
        {
            throw RegistryError{ "Failed to write to registry key", (LONG)setValueStatus };
        }
    }

    void DeleteTree(HKEY hkey, const std::wstring& subkey)
    {
        LSTATUS status = RegDeleteTreeW(
            hkey,
            subkey.c_str()
        );
        if (status != ERROR_SUCCESS)
            throw RegistryError{ "Failed to write to registry key", (LONG)status };
    }
}