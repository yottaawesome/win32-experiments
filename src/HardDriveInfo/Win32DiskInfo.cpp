#include "Header.hpp"
#include <iostream>

// https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-diskdrive
int Win32DiskInfo()
{
	WmiProxy wmiProxy;
	WmiServer server = wmiProxy.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator objectEnum = server.Query(L"select * from Win32_DiskDrive");

	while (IWbemClassObject* pClsObj = objectEnum.Next())
	{
		WmiClassObject clsObj(pClsObj);
		std::wcout << "Manufacturer: " << clsObj.String(L"Manufacturer") << std::endl;
	}

	return 0;
}
