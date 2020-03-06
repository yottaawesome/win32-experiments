#pragma once
#include "Header.hpp"
#include <iostream>

int MsftDiskInfo()
{
	WmiProxy wmi;
	WmiServer rootCim2 = wmi.ConnectServer(L"ROOT\\MICROSOFT\\WINDOWS\\STORAGE");
	WmiObjectEnumerator enumerator = rootCim2.Query(L"SELECT * FROM MSFT_PhysicalDisk");

	while (IWbemClassObject* ptrClsObj = enumerator.Next())
	{
		WmiClassObject classObj(ptrClsObj);

		std::wcout << L" Capacity : " << classObj.String(L"Capacity") << std::endl;
		std::wcout << L" FriendlyName : " << classObj.String(L"FriendlyName") << std::endl;
	}

	return 0;
}