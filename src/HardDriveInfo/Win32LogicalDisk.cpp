#include "Header.hpp"
#include <iostream>
#include <WTypes.h>
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

int Win32LogicalDisk()
{
	WmiProxy wmi;
	WmiServer rootCim2 = wmi.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator enumerator = rootCim2.Query(L"SELECT * FROM Win32_LogicalDisk");

	while (IWbemClassObject* ptrClsObj = enumerator.Next())
	{
		WmiClassObject classObj(ptrClsObj);

		std::wcout << L" DeviceID : " << classObj.String(L"DeviceID") << std::endl;
		std::wcout << L" FileSystem : " << classObj.String(L"FileSystem") << std::endl;
		std::wcout << L" DriveType : " << classObj.Int32(L"DriveType") << std::endl;
		std::wcout << L" MediaType : " << classObj.Int32(L"MediaType") << std::endl;
		std::wcout << L" Name : " << classObj.String(L"Name") << std::endl;
		std::wcout << L" Size : " << classObj.StringAsUInt64(L"Size") << std::endl;
		std::wcout << L" FreeSpace : " << classObj.StringAsUInt64(L"FreeSpace") << std::endl;
	}

	return 0;
}
