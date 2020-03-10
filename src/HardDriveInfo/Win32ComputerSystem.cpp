#include "Header.hpp"

// https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-computersystem
int Win32ComputerSystem()
{
	WmiProxy wmiProxy;
	WmiServer server = wmiProxy.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator objectEnum = server.Query(L"select * from Win32_ComputerSystem");

	while (IWbemClassObject* wbemClassObj = objectEnum.Next())
	{
		WmiClassObject classObj(wbemClassObj);

		Log(L"HypervisorPresent", classObj.Bool(L"HypervisorPresent"));
		Log(L"PartOfDomain", classObj.Bool(L"PartOfDomain"));
		Log(L"Domain", classObj.String(L"Domain"));
		Log(L"Workgroup", classObj.String(L"Workgroup"));
		Log(L"DNSHostName", classObj.String(L"DNSHostName"));
		Log(L"Name", classObj.String(L"Name"));
		Log(L"Model", classObj.String(L"Model"));
		Log(L"Manufacturer", classObj.String(L"Manufacturer"));
		Log(L"Status", classObj.String(L"Status"));
		Log(L"SystemFamily", classObj.String(L"SystemFamily"));
		Log(L"NumberOfProcessors", classObj.Int32(L"NumberOfProcessors"));
		Log(L"NumberOfLogicalProcessors", classObj.Int32(L"NumberOfLogicalProcessors"));
		Log(L"DomainRole", classObj.Int32(L"DomainRole"));
	}

	return 0;
}
