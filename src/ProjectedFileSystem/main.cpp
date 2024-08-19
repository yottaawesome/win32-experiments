#pragma comment(lib, "projectedfslib.lib")
// https://scorpiosoftware.net/2024/02/20/projected-file-system/

import std;
import common;
import projectedfilesystem;

bool operator==(const Win32::GUID g1, const Win32::GUID g2)
{
    return Win32::IsEqualGUID(g1, g2);
}

std::wstring m_RootDir = L"";
Win32::PRJ_NAMESPACE_VIRTUALIZATION_CONTEXT m_VirtContext;

int main() try
{
    projected_file_system::pfs_context context(".\\test");
    return 0;
}
catch (const std::exception& ex)
{
    std::println("An exception occurred: {}", ex.what());
}
