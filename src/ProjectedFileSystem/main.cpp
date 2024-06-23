#pragma comment(lib, "projectedfslib.lib")
// https://scorpiosoftware.net/2024/02/20/projected-file-system/

import std;
import win32;

bool operator==(const Win32::GUID g1, const Win32::GUID g2)
{
    return Win32::IsEqualGUID(g1, g2);
}

struct HResult
{
    HResult(Win32::HRESULT hr)
        : m_hr(hr)
    { }

    Win32::HRESULT m_hr;

    operator bool() const noexcept
    {
        return Win32::HrSucceeded(m_hr);
    }

    operator Win32::HRESULT() const noexcept
    {
        return m_hr;
    }
};

std::wstring m_RootDir = L"";
Win32::PRJ_NAMESPACE_VIRTUALIZATION_CONTEXT m_VirtContext;

void Init(Win32::PCWSTR root)
{
    Win32::GUID instanceId = Win32::NullGuid;
    std::wstring instanceFile(root);
    instanceFile += L"\\_obgmgrproj.guid";

    if (!Win32::CreateDirectoryW(root, nullptr))
    {
        if (Win32::GetLastError() != Win32::Error::AlreadyExists)
            std::terminate();

        Win32::HANDLE hFile = Win32::CreateFileW(
            instanceFile.c_str(), 
            Win32::GenericRead,
            Win32::ShareRead,
            nullptr, 
            Win32::OpenExisting,
            0, 
            nullptr
        );
        if (hFile != Win32::InvalidHandleValue && Win32::GetFileSize(hFile, nullptr) == sizeof(Win32::GUID)) 
        {
            Win32::DWORD ret;
            Win32::ReadFile(hFile, &instanceId, sizeof(instanceId), &ret, nullptr);
            Win32::CloseHandle(hFile);
        }
    }

    if (instanceId == Win32::NullGuid) 
    {
        Win32::CoCreateGuid(&instanceId);
        //
        // write instance ID
        //
        auto hFile = Win32::CreateFileW(
            instanceFile.c_str(), 
            Win32::GenericWrite,
            0, 
            nullptr, 
            Win32::CreateNew,
            Win32::FileAttributeHidden,
            nullptr
        );
        if (hFile != Win32::InvalidHandleValue) 
        {
            Win32::DWORD ret;
            Win32::WriteFile(
                hFile, 
                &instanceId, 
                sizeof(instanceId), 
                &ret, 
                nullptr
            );
            Win32::CloseHandle(hFile);
        }
    }

    HResult hr = Win32::PrjMarkDirectoryAsPlaceholder(
        root, 
        nullptr, 
        nullptr, 
        &instanceId
    );
    if (not hr)
        std::terminate();

    Win32::PRJ_CALLBACKS cb{};
    //cb.StartDirectoryEnumerationCallback = StartDirectoryEnumerationCallback;
    //cb.EndDirectoryEnumerationCallback = EndDirectoryEnumerationCallback;
    //cb.GetDirectoryEnumerationCallback = GetDirectoryEnumerationCallback;
    //cb.GetPlaceholderInfoCallback = GetPlaceholderInformationCallback;
    //cb.GetFileDataCallback = GetFileDataCallback;

    hr = Win32::PrjStartVirtualizing(
        m_RootDir.c_str(), 
        &cb, 
        nullptr,//this, 
        nullptr, 
        &m_VirtContext
    );
}

int main()
{
    return 0;
}
