#include <Windows.h>
#include <iostream>
#include "ClientServerLib/include/ClientServerLib.hpp"

#define BUF_SIZE 256
int main()
{
	//ClientServerLib::IntArray mmf(L"MyFileMappingObject", 512 * sizeof(wchar_t), false);
	ClientServerLib::TypedArray<ClientServerLib::Message> mmf(L"MyFileMappingObject", false);
	std::wcout << L"Client opened handle!" << std::endl;

	// Give time for the main process to populate the data.
	Sleep(5000);

	while (mmf.GetCurrentCount() < 10)
		Sleep(1000);

	for (int i = 0; i < 10; i++)
	{
		mmf.Lock();
		ClientServerLib::Message* msg = mmf[i];
		std::wcout << L"Client read: " << msg->GetMsg() << std::endl;
		//std::wcout << mmf.GetCurrentCount() << std::endl;
		mmf.Unlock();
	}

	std::wcout << L"Client exited!" << std::endl;

	return 0;
}
