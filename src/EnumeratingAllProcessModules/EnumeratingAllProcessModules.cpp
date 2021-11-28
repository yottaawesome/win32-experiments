#include <iostream>
#include <format>
#include <string>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <psapi.h>

// https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocessmodules
int PrintModules(DWORD processID)
{
    std::wcout << "Process ID: " << processID << std::endl;

    // Get a handle to the process.
    HANDLE hProcess;
    hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        false, 
        processID
    );
    if (!hProcess)
        return 1;

    // Get a list of all the modules in this process.
    HMODULE hMods[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            std::wstring szModName(MAX_PATH, '\0');
            // Get the full path to the module's file.
            // https://docs.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getmodulefilenameexw
            if (GetModuleFileNameEx(hProcess, hMods[i], &szModName[0], szModName.size()))
            {
                std::wcout << std::format(L"{}\n", szModName);
                //_tprintf(TEXT("\t%s (0x%08X)\n"), szModName, hMods[i]);
            }
        }
    }

    // Release the handle to the process.
    CloseHandle(hProcess);

    return 0;
}

int AllProcesses()
{
    DWORD aProcesses[1024];
    DWORD cbNeeded;
    DWORD cProcesses;
    unsigned int i;

    // Get the list of process identifiers.
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
        return 1;

    // Calculate how many process identifiers were returned.

    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the names of the modules for each process.

    for (i = 0; i < cProcesses; i++)
    {
        PrintModules(aProcesses[i]);
    }
    return 1;
}

int main(int argc, char* args[])
{
    PrintModules(GetCurrentProcessId());
    return 0;
}
