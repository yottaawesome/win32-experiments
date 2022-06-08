#include <iostream>
#include <format>
#include <stdexcept>
#include <memory>
#include <source_location>
#include <Windows.h>
#include <wlanapi.h>
#include <wtypes.h>

#pragma comment(lib, "wlanapi.lib")

struct WlanHandleDeleter
{
    void operator()(HANDLE handle)
    {
        WlanCloseHandle(handle, 0);
    }
};
using UniquePtrWlanHandle = std::unique_ptr<std::remove_pointer<HANDLE>::type, WlanHandleDeleter>;

struct WlanMemorDeleter
{
    void operator()(void* handle)
    {
        WlanFreeMemory(handle);
    }
};
using UniquePtrWlanMemory = std::unique_ptr<void, WlanMemorDeleter>;

std::wstring GuidToStringW(const GUID& guid)
{
    std::wstring stringGuid(64, '\0');
    size_t numberOfChars = StringFromGUID2(guid, &stringGuid[0], 64);
    if (!numberOfChars)
        throw std::runtime_error("StringFromGUID2() failed");
    stringGuid.resize(numberOfChars - 1);
    return stringGuid;
}

std::string ConvertString(const std::wstring_view wstr)
{
    if (wstr.empty())
        return "";

    // https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte
    // Returns the size in bytes, this differs from MultiByteToWideChar, which returns the size in characters
    const int sizeInBytes = WideCharToMultiByte(
        CP_UTF8,										// CodePage
        WC_NO_BEST_FIT_CHARS,							// dwFlags 
        &wstr[0],										// lpWideCharStr
        static_cast<int>(wstr.size()),					// cchWideChar 
        nullptr,										// lpMultiByteStr
        0,												// cbMultiByte
        nullptr,										// lpDefaultChar
        nullptr											// lpUsedDefaultChar
    );
    if (sizeInBytes == 0)
        throw std::runtime_error("WideCharToMultiByte() [1] failed");

    std::string strTo(sizeInBytes / sizeof(char), '\0');
    const int status = WideCharToMultiByte(
        CP_UTF8,										// CodePage
        WC_NO_BEST_FIT_CHARS,							// dwFlags 
        &wstr[0],										// lpWideCharStr
        static_cast<int>(wstr.size()),					// cchWideChar 
        &strTo[0],										// lpMultiByteStr
        static_cast<int>(strTo.size() * sizeof(char)),	// cbMultiByte
        nullptr,										// lpDefaultChar
        nullptr											// lpUsedDefaultChar
    );
    if (status == 0)
        throw std::runtime_error("WideCharToMultiByte() [2] failed");

    return strTo;
}

std::wstring ConvertString(const std::string_view str)
{
    if (str.empty())
        return L"";

    // https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
    // Returns the size in characters, this differs from WideCharToMultiByte, which returns the size in bytes
    const int sizeInCharacters = MultiByteToWideChar(
        CP_UTF8,									// CodePage
        0,											// dwFlags
        &str[0],									// lpMultiByteStr
        static_cast<int>(str.size() * sizeof(char)),// cbMultiByte
        nullptr,									// lpWideCharStr
        0											// cchWideChar
    );
    if (sizeInCharacters == 0)
        throw std::runtime_error("MultiByteToWideChar() [1] failed");

    std::wstring wstrTo(sizeInCharacters, '\0');
    const int status = MultiByteToWideChar(
        CP_UTF8,									// CodePage
        0,											// dwFlags
        &str[0],									// lpMultiByteStr
        static_cast<int>(str.size() * sizeof(char)),	// cbMultiByte
        &wstrTo[0],									// lpWideCharStr
        static_cast<int>(wstrTo.size())				// cchWideChar
    );
    if (status == 0)
        throw std::runtime_error("MultiByteToWideChar() [2] failed");

    return wstrTo;
}

int main(int argc, char* argv)
{
    // Open a WLAN session
    DWORD maxClientVersion = 2;
    DWORD currentVersion = 0;
    HANDLE wlanHandle = nullptr;
    // https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanopenhandle
    DWORD status = WlanOpenHandle(
        maxClientVersion, 
        nullptr, 
        &currentVersion, 
        &wlanHandle
    );
    if (status != ERROR_SUCCESS)
    {
        std::wcerr << std::format(L"WlanOpenHandle() format: {}\n", status);
        return status;
    }
    UniquePtrWlanHandle wlanSession = UniquePtrWlanHandle(wlanHandle);

    // Enumerate the interface list
    PWLAN_INTERFACE_INFO_LIST pIfList = nullptr;
    // https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanenuminterfaces
    status = WlanEnumInterfaces(wlanSession.get(), nullptr, &pIfList);
    if (status != ERROR_SUCCESS)
    {
        std::wcerr << std::format(L"WlanEnumInterfaces() format: {}\n", status);
        return status;
    }
    UniquePtrWlanMemory interfaceList = UniquePtrWlanMemory(pIfList);

    for (DWORD index = 0; index < pIfList->dwNumberOfItems; index++)
    {
        if (pIfList->InterfaceInfo[index].isState != wlan_interface_state_connected)
            continue;

        PWLAN_INTERFACE_INFO interfaceInfo = &pIfList->InterfaceInfo[index];

        // Try to get the connection attributes
        PWLAN_CONNECTION_ATTRIBUTES pConnectionAttributes = nullptr;
        DWORD connectAttrSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
        WLAN_OPCODE_VALUE_TYPE opCode = wlan_opcode_value_type_invalid;
        status = WlanQueryInterface(
            wlanSession.get(),
            &interfaceInfo->InterfaceGuid,
            wlan_intf_opcode_current_connection,
            nullptr,
            &connectAttrSize,
            reinterpret_cast<PVOID*>(&pConnectionAttributes),
            &opCode
        );
        UniquePtrWlanMemory connectionAttributes;
        if (status == ERROR_SUCCESS)
        {
            std::string ssid = std::string(reinterpret_cast<char*>(pConnectionAttributes->wlanAssociationAttributes.dot11Ssid.ucSSID));
            connectionAttributes = UniquePtrWlanMemory(pConnectionAttributes);
            std::wcerr << std::format(
                L"Signal Quality on {} (ssid {}) is {}\n",
                GuidToStringW(interfaceInfo->InterfaceGuid),
                ConvertString(ssid),
                pConnectionAttributes->wlanAssociationAttributes.wlanSignalQuality
            );
        }
        else
        {
            std::wcerr << std::format(
                L"WlanQueryInterface() failed on index {} with status: {}\n",
                index,
                status
            );
        }
        
        PWLAN_BSS_LIST pWlanBssList = nullptr;
        // https://docs.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlangetnetworkbsslist
        status = WlanGetNetworkBssList(
            wlanSession.get(),
            &interfaceInfo->InterfaceGuid,
            nullptr,
            DOT11_BSS_TYPE::dot11_BSS_type_any,
            false,
            nullptr,
            &pWlanBssList
        );
        if (status != ERROR_SUCCESS)
        {
            std::wcerr << std::format(
                L"WlanGetNetworkBssList() failed on index {} with status: {}\n", 
                index, 
                status
            );
            continue;
        }
        if (!pWlanBssList)
            continue;
        UniquePtrWlanMemory wlanBssList = UniquePtrWlanMemory(pWlanBssList);

        std::wcout << std::format(L"Found {} BSS entries for interface {}.\n", 
            pWlanBssList->dwNumberOfItems,
            GuidToStringW(interfaceInfo->InterfaceGuid)
        );
        for (DWORD bssIndex = 0; bssIndex < pWlanBssList->dwNumberOfItems; bssIndex++)
        {
            WLAN_BSS_ENTRY* bssEntry = &pWlanBssList->wlanBssEntries[bssIndex];            
            std::wcerr << std::format(
                L"BSS uLinkQuality: {}\n",
                bssEntry->uLinkQuality
            );
        }
    }

    return 0;
}
