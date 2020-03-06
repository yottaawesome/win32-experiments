#include "Header.hpp"
#include <iostream>
#include <WTypes.h>
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

int Win32RamInfo()
{
	// https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-physicalmemory
	WmiProxy wmi;
	WmiServer rootCim2 = wmi.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator enumerator = rootCim2.Query(L"SELECT * FROM Win32_PhysicalMemory");

	while (IWbemClassObject* ptrClsObj = enumerator.Next())
	{
		WmiClassObject classObj(ptrClsObj);

		std::wstring capacity = classObj.String(L"Capacity");
		std::wcout << L" Capacity : " << capacity << std::endl;
	}

	return 0;
}
