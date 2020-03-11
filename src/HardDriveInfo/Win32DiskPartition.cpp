#include "Header.hpp"

int Win32DiskPartition()
{
	WmiProxy wmiProxy;
	WmiServer server = wmiProxy.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator objectEnum = server.Query(L"select * from Win32_DiskPartition");

	while (IWbemClassObject* wbemClassObj = objectEnum.Next())
	{
		WmiClassObject classObj(wbemClassObj);

		Log(L"Size", classObj.String(L"Size"));
		Log(L"Status", classObj.String(L"Status"));
		Log(L"DiskIndex", classObj.Int32(L"DiskIndex"));
		Log(L"Type", classObj.String(L"Type"));
		Log(L"Index", classObj.Int32(L"Index"));
		//Log(L"PowerOnHours", classObj.Int32(L"PowerOnHours"));
		Log(L"PrimaryPartition", classObj.Bool(L"PrimaryPartition"));
		Log(L"BootPartition", classObj.Bool(L"BootPartition"));
		Log(L"Bootable", classObj.Bool(L"Bootable"));
	}

	return 0;
}