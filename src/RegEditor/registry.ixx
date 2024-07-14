export module registry;
import common;

export namespace Registry
{
    template<typename T, typename...S>
    concept OneOf = (std::same_as<T, S> or ...);

    class RegistryError : public std::runtime_error
    {
        public:
        RegistryError(std::string_view message)
            : std::runtime_error{ message.data() }
        {}

        RegistryError(const char* message, Win32::LONG errorCode)
            : std::runtime_error{ message }, m_errorCode{ errorCode }
        {}

        Win32::LONG ErrorCode() const noexcept
        {
            return m_errorCode;
        }

        private:
        Win32::LONG m_errorCode;
    };

    // Maps C++ types to registry types.
    template<typename T>
    struct RegistryValueMap {};
    template<>
    struct RegistryValueMap<std::wstring> { static constexpr int Type = Win32::Registry::ValueTypes::String; };
    template<>
    struct RegistryValueMap<Win32::DWORD> { static constexpr int Type = Win32::Registry::ValueTypes::DWORD; };
    template<>
    struct RegistryValueMap<size_t> { static constexpr int Type = Win32::Registry::ValueTypes::QWORD; };

    // Maps return values of Get functions. Necessary for std::expected support.
    template<typename T, bool VThrowOnError>
    struct RValue { };
    template<typename T>
    struct RValue<T, true> { using Type = T; };
    template<typename T>
    struct RValue<T, false> { using Type = std::expected<T, Win32::LONG>; };

    template<OneOf<std::wstring, Win32::DWORD, size_t> T, bool VThrowOnError = true>
    RValue<T, VThrowOnError>::Type Get(
        Win32::Registry::HKEY hKey,
        std::wstring_view subKey,
        std::wstring_view value
    )
    {
        T data;
        Win32::DWORD dataSize;
        Win32::PVOID arg = nullptr;
        Win32::LONG statusCode;

        if constexpr (std::same_as<std::wstring, T>)
        {
            statusCode = Win32::Registry::RegGetValueW(
                hKey,
                subKey.data(),
                value.data(),
                Win32::Registry::ValueTypes::String,
                nullptr,
                nullptr,
                &dataSize
            );
            if (statusCode != Win32::Error::Success)
                if constexpr (VThrowOnError)
                    throw RegistryError{ "Cannot read value from registry.", statusCode };
                else
                    return std::unexpected(statusCode);

            data.resize(dataSize / sizeof(wchar_t), '\0');
            arg = data.data();
        }
        else
        {
            dataSize = sizeof(T);
            arg = &data;
        }

        statusCode = Win32::Registry::RegGetValueW(
            hKey,
            subKey.data(),
            value.data(),
            RegistryValueMap<T>::Type,
            nullptr,
            arg,
            &dataSize
        );
        if (statusCode != Win32::Error::Success)
            if constexpr (VThrowOnError)
                throw RegistryError{ "Cannot read value from registry.", statusCode };
            else
                return std::unexpected(statusCode);

        // Remove additional NUL
        if constexpr (std::same_as<std::wstring, T>)
            data.pop_back();

        return data;
    }

    bool CreateKey(
        Win32::Registry::HKEY hKey,
        const std::wstring& subKey
    )
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regcreatekeyexw
        Win32::DWORD disposition;
        Win32::Registry::HKEY openedKey;
        Win32::DWORD createKeyStatus = Win32::Registry::RegCreateKeyExW(
            hKey,
            subKey.c_str(),
            0,
            nullptr,
            Win32::Registry::Options::NonVolatile,
            Win32::Registry::Access::AllAccess,
            nullptr,
            &openedKey,
            &disposition
        );
        if (createKeyStatus != Win32::Error::Success)
            throw RegistryError{ "Failed to write to registry", (Win32::LONG)createKeyStatus };

        Win32::Registry::RegCloseKey(openedKey);
        return disposition == Win32::Registry::Disposition::NewKey;
    }

    template<typename T>
    void Set(
        const Win32::Registry::HKEY parent,
        std::wstring_view subkey,
        std::wstring_view valueName,
        const T& value
    )
    {
        if (not parent)
            throw RegistryError("key is nullptr");

        Win32::DWORD size = 0;
        Win32::LPCVOID arg = nullptr;
        if constexpr (std::same_as<T, std::wstring>)
        {
            size = static_cast<Win32::DWORD>(value.size()) * sizeof(wchar_t);
            arg = value.data();
        }
        else
        {
            size = sizeof(T);
            arg = reinterpret_cast<Win32::LPCVOID>(const_cast<T*>(&value));
        }

        const Win32::LSTATUS status = Win32::Registry::RegSetKeyValueW(
            parent,
            subkey.data(),
            valueName.data(),
            RegistryValueMap<T>::Type,
            arg,
            size
        );
        if (status != Win32::Error::Success)
            throw RegistryError("RegSetValueExW() failed", status);
    }

    bool DoesKeyExist(
        Win32::Registry::HKEY hKey,
        std::wstring_view subKey
    )
    {
        Win32::Registry::HKEY openedKey = nullptr;
        Win32::LSTATUS result = Win32::Registry::RegOpenKeyExW(
            hKey, 
            subKey.data(), 
            0, 
            Win32::Registry::Permissions::Read, 
            &openedKey
        );
        if (openedKey)
            Win32::Registry::RegCloseKey(openedKey);

        return result == Win32::Error::Success;
    }
}