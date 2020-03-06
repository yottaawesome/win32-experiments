#include "Header.hpp"
#include <iostream>
#include <WTypes.h>
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

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
}

