#include "Header.hpp"


int Win32VideoController()
{
	WmiProxy wmiProxy;
	WmiServer server = wmiProxy.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator objectEnum = server.Query(L"select * from Win32_VideoController");

	while (IWbemClassObject* wbemClassObj = objectEnum.Next())
	{
		WmiClassObject classObj(wbemClassObj);

		Log(L"Name", classObj.String(L"Name"));
		Log(L"AdapterDACType", classObj.String(L"AdapterDACType"));
		Log(L"AdapterCompatibility", classObj.String(L"AdapterCompatibility"));
		Log(L"AdapterRAM", classObj.Int32(L"AdapterRAM"));
		Log(L"CurrentVerticalResolution", classObj.Int32(L"CurrentVerticalResolution"));
		Log(L"CurrentHorizontalResolution", classObj.Int32(L"CurrentHorizontalResolution"));
	}

	return 0;
}