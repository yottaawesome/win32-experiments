#include "Header.hpp"
#include "iostream"

int Win32Bios()
{
	WmiProxy wmiProxy;
	WmiServer server = wmiProxy.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator objectEnum = server.Query(L"select * from Win32_BIOS");

	while (IWbemClassObject* pClsObj = objectEnum.Next())
	{
		WmiClassObject clsObj(pClsObj);

		Log(L"Manufacturer", clsObj.String(L"Manufacturer"));
		Log(L"Name", clsObj.String(L"Name"));
		Log(L"BuildNumber", clsObj.String(L"BuildNumber"));
		Log(L"PrimaryBios", clsObj.Bool(L"PrimaryBios"));
		Log(L"Version", clsObj.String(L"Version"));
		Log(L"SMBIOSBIOSVersion", clsObj.String(L"SMBIOSBIOSVersion"));
		Log(L"Status", clsObj.String(L"Status"));
		Log(L"SerialNumber", clsObj.String(L"SerialNumber"));
	}

	return 0;
}