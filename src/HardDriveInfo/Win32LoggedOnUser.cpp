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

		std::wstring antecedent = classObj.String(L"Antecedent");
		WmiClassObject aClsObj(server.GetClassObject(antecedent.c_str()));
		std::wstring dependendent = classObj.String(L"Dependent");
		WmiClassObject dClsObj(server.GetClassObject(dependendent.c_str()));

		Log(L"SID", aClsObj.String(L"SID"));
		Log(L"Name", aClsObj.String(L"Name"));
		Log(L"Status", aClsObj.String(L"Status"));
		Log(L"Domain", aClsObj.String(L"Domain"));
		Log(L"SIDType", aClsObj.Byte(L"SIDType"));
		Log(L"StartTime", dClsObj.String(L"StartTime"));
		Log(L"LogonType", dClsObj.Int32(L"LogonType"));
		//Log(L"LocalAccount", dClsObj.Bool(L"LocalAccount"));
	}

	return 0;
}