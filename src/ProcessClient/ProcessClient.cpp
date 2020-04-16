#include <Windows.h>
#include <iostream>

#define BUF_SIZE 256
int main()
{
	TCHAR szName[] = TEXT("MyFileMappingObject");
	HANDLE hMapFile = 
		OpenFileMapping(
			FILE_MAP_ALL_ACCESS,   // read/write access
			true,                 // do not inherit the name
			szName					// name of mapping object
		);

	if (hMapFile == nullptr)
	{
		std::wcout << L"Client failed to open handle!" << std::endl;
		return 1;
	}

	std::wcout << L"Client opened handle!" << std::endl;
	LPCTSTR pBuf;
	pBuf = (LPTSTR)
		MapViewOfFile(
			hMapFile, // handle to map object
			FILE_MAP_ALL_ACCESS,  // read/write permission
			0,
			0,
			BUF_SIZE
		);
	std::wcout << pBuf << std::endl;
	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);

	std::wcout << L"Client exited!" << std::endl;
}
