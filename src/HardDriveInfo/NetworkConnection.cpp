#include "Header.hpp"

int Win32NetworkConnection()
{
	WmiProxy wmiProxy;
	WmiServer server = wmiProxy.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator objectEnum = server.Query(L"select * from Win32_NetworkConnection");

	while (IWbemClassObject* wbemClassObj = objectEnum.Next())
	{
		WmiClassObject classObj(wbemClassObj);

		Log(L"ConnectionType", classObj.String(L"ConnectionType"));
		Log(L"Status", classObj.String(L"Status"));
		Log(L"Name", classObj.String(L"Name"));
		Log(L"ConnectionState", classObj.String(L"ConnectionState"));
		Log(L"LocalName", classObj.String(L"LocalName"));
	}

	return 0;
}