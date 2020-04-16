#include <iostream>
#include <Windows.h>
#include <string>
#include <conio.h>
#include <tchar.h>

#define BUF_SIZE 256

int main(int argc, TCHAR* argv[])
{
	std::wstring buffer;
	buffer.resize(150);

	GetModuleFileName(nullptr, &buffer[0], buffer.size());
	buffer.shrink_to_fit();
	std::wcout << buffer << std::endl;

	STARTUPINFO si{ 0 };
	PROCESS_INFORMATION pi{ 0 };
	si.cb = sizeof(si);

	std::wstring filePath;
	filePath.resize(250);
	GetModuleFileName(nullptr, &filePath[0], filePath.size());
	filePath.shrink_to_fit();
	std::wcout << filePath << std::endl;

	TCHAR szName[] = TEXT("MyFileMappingObject");
	HANDLE hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		BUF_SIZE,                // maximum object size (low-order DWORD)
		szName);                 // name of mapping object
	if (hMapFile)
	{
		std::cout << "Server opened handle!" << std::endl;
		LPCTSTR pBuf;
		pBuf = (LPTSTR)MapViewOfFile(hMapFile,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,
			0,
			BUF_SIZE);
		TCHAR szMsg[] = TEXT("Message from first process.");
		CopyMemory((PVOID)pBuf, szMsg, (_tcslen(szMsg) * sizeof(TCHAR)));
		UnmapViewOfFile(pBuf);
	}
	else
	{
		std::cout << "Server failed to open handle!" << std::endl;
	}

	// https://docs.microsoft.com/en-us/windows/win32/procthread/creating-processes
	bool successfullyCreatedUpdateProcess =
		CreateProcess(
			L"A:\\Code\\C++\\win32-experiments\\src\\x64\\Debug\\ProcessClient.exe",	// Module name
			nullptr,				// Command line
			nullptr,				// Process handle not inheritable
			nullptr,				// Thread handle not inheritable
			true,					// Set handle inheritance to FALSE
			0,						// No creation flags
			nullptr,				// Use parent's environment block
			nullptr,				// Use parent's starting directory 
			&si,					// Pointer to STARTUPINFO structure
			&pi						// Pointer to PROCESS_INFORMATION structure
		);

	if (successfullyCreatedUpdateProcess)
	{
		// Close process and thread handles. 
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		std::wcout << "OK" << std::endl;
	}
	else
	{
		std::wcout << "Failed." << GetLastError() << std::endl;
	}

	Sleep(10000);
	if (hMapFile)
	{
		CloseHandle(hMapFile);
	}

	return 0;
}
