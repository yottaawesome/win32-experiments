#include <iostream>
#include <format>
#include <string>
#include <memory>
#include <stdexcept>
#include <vector>
#include <windows.h>
#include <winnt.h>
#include <tchar.h>
#include <stdio.h>
#include <psapi.h>

struct HandleDeleter final
{
    void operator()(HANDLE handle) { CloseHandle(handle); }
};
using HandleUniquePtr = std::unique_ptr<std::remove_pointer<HANDLE>::type, HandleDeleter>;

template<typename...Args>
void Log(const std::wstring_view str, const Args&... arg)
{
    std::wcout << std::vformat(str, std::make_wformat_args(arg...)) << std::endl;
}

void PrintProcessTime(const DWORD processID)
{
    HandleUniquePtr hProcess(OpenProcess(
        PROCESS_QUERY_INFORMATION,
        false,
        processID
    ));
    if (!hProcess)
    {
        Log(L"OpenProcess() failed:  {}", GetLastError());
        return;
    }

    FILETIME ftCreationTime{ 0 };
    FILETIME ftExitTime{ 0 };
    FILETIME ftKernelTime{ 0 };
    FILETIME ftUserTime{ 0 };

    const bool success = GetProcessTimes(
        hProcess.get(), 
        &ftCreationTime, 
        &ftExitTime, 
        &ftKernelTime, 
        &ftUserTime
    );
    if (!success)
    {
        Log(L"GetProcessTimes() failed:  {}", GetLastError());
        return;
    }
    size_t kernelTime = ULARGE_INTEGER{ ftKernelTime.dwLowDateTime, ftKernelTime.dwHighDateTime }.QuadPart;
    size_t userTime = ULARGE_INTEGER{ ftUserTime.dwLowDateTime, ftUserTime.dwHighDateTime }.QuadPart;
    Log(L"Process ID: {}, process time: {}", processID, userTime + kernelTime);
}

// https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocessmodules
int PrintModules(DWORD processID)
{
    std::wcout << "Process ID: " << processID << std::endl;

    // Get a handle to the process.
    HandleUniquePtr hProcess(OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        false, 
        processID
    ));
    if (!hProcess)
        return 1;

    // Resize our modules vector to fit all proc modules
    std::vector<HMODULE> modules(1024);
    while (true)
    {
        const DWORD modulesByteSize = modules.size() * sizeof(HMODULE);
        DWORD bytesNeeded;
        // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocessmodules
        if (!EnumProcessModules(hProcess.get(), &modules[0], modulesByteSize, &bytesNeeded))
            throw std::runtime_error("EnumProcessModules() failed");
        modules.resize(bytesNeeded / sizeof(HMODULE));
        if (bytesNeeded <= modulesByteSize)
            break;
    }

    for (const auto& procModule : modules)
    {
        std::wstring moduleName(MAX_PATH, '\0');
        // Get the full path to the module's file.
        // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getmodulefilenameexw
        if (!GetModuleFileNameEx(hProcess.get(), procModule, &moduleName[0], moduleName.size()))
            throw std::runtime_error("GetModuleFileNameEx() failed");
        std::wcout << std::format(L"\t{}\n", moduleName);
    }

    return 0;
}

void AllProcesses()
{
    // Get the list of process identifiers.
    DWORD aProcesses[1024];
    DWORD cbNeeded;
    // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocesses
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
        throw std::runtime_error("EnumProcesses() failed");

    // Calculate how many process identifiers were returned.
    DWORD cProcesses = cbNeeded / sizeof(DWORD);

    // Print the names of the modules for each process.
    for (const DWORD process : aProcesses) try
    {
        PrintProcessTime(process);
        //PrintModules(process);
    }
    catch (const std::exception& ex)
    {
        std::wcerr << ex.what() << std::endl;
    }
}

int main(int argc, char* args[])
{
    //__fastfail(FAST_FAIL_UNSAFE_REGISTRY_ACCESS);

    try
    {
        AllProcesses();
    }
    catch (const std::exception& ex)
    {
        std::wcerr << ex.what() << std::endl;
    }
    //PrintModules(GetCurrentProcessId());
    return 0;
}
