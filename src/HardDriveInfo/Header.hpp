#pragma once
#include <string>

int MsftDiskInfo();
int Win32DiskInfo();
int Win32RamInfo();

class Wmi
{
	public:
		Wmi();
		~Wmi();
		void ConnectServer(std::wstring server);
};
