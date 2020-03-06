#include "Header.hpp"

// https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-loggedonuser
int Win32LoggedOnUser()
{
	WmiProxy wmiProxy;
	WmiServer server = wmiProxy.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator objectEnum = server.Query(L"select * from Win32_LoggedOnUser");

	while (IWbemClassObject* wbemClassObj = objectEnum.Next())
	{
		WmiClassObject classObj(wbemClassObj);
	}
}