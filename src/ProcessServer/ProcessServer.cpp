#include <iostream>
#include <Windows.h>
#include <string>
#include <conio.h>
#include <tchar.h>
#include "ClientServerLib/include/ClientServerLib.hpp"

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

	//ClientServerLib::IntArray mmf(L"MyFileMappingObject", 512 * sizeof(int), true);
	ClientServerLib::TypedArray<ClientServerLib::Message> mmf(L"MyFileMappingObject", true);

	// https://docs.microsoft.com/en-us/windows/win32/procthread/creating-processes
	bool successfullyCreatedChild =
		CreateProcess(
			L"A:\\Code\\C++\\win32-experiments\\src\\x64\\Debug\\ProcessClient.exe",	// Module m_name
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

	if (successfullyCreatedChild)
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
	
	for (int i = 0; i < 10; i++)
	{
		std::wcout << L"iteration: " << i << std::endl;
		mmf.Add(L"Hello no. " + std::to_wstring(i));
		//mmf.SetAt(i,i);
		//std::wstring msg(L"Message number: ");
		//msg += (std::to_wstring(i));
		//mmf.Write(msg);
		Sleep(1000);
	}

	for (int i = 0; i < 10; i++)
	{
		ClientServerLib::Message* msg = mmf[i];
		std::wcout << msg->GetMsg() << std::endl;
	}

	Sleep(15000);

	return 0;
}
