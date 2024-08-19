#pragma comment(lib, "projectedfslib.lib")
// https://scorpiosoftware.net/2024/02/20/projected-file-system/

import std;
import common;
import projectedfilesystem;

std::wstring m_RootDir = L"";

int main() try
{
    projected_file_system::pfs_context context(".\\test");
    return 0;
}
catch (const std::exception& ex)
{
    std::println("An exception occurred: {}", ex.what());
}
