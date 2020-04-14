#include <iostream>
#include <Windows.h>
#include <string>

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

	// https://docs.microsoft.com/en-us/windows/win32/procthread/creating-processes
	bool successfullyCreatedUpdateProcess =
		CreateProcess(
			L"A:\\Code\\C++\\win32-experiments\\src\\x64\\Debug\\ProcessClient.exe",	// Module name
			nullptr,				// Command line
			nullptr,				// Process handle not inheritable
			nullptr,				// Thread handle not inheritable
			false,					// Set handle inheritance to FALSE
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

	return 0;
}
