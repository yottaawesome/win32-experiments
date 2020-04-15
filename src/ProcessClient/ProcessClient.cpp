#include <Windows.h>
#include <iostream>

int main()
{
	TCHAR szName[] = TEXT("MyFileMappingObject");
	HANDLE hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		true,                 // do not inherit the name
		szName);               // name of mapping object
	if (hMapFile)
	{
		std::cout << "Client opened handle!" << std::endl;
		CloseHandle(hMapFile);
	}
	else
	{
		std::cout << "Client failed to open handle!" << std::endl;
	}

	std::cout << "Client exited!" << std::endl;
}
