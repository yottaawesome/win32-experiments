#include "Header.hpp"

int Win32OperatingSystem()
{
	WmiProxy wmiProxy;
	WmiServer server = wmiProxy.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator objectEnum = server.Query(L"select * from Win32_OperatingSystem");

	while (IWbemClassObject* wbemClassObj = objectEnum.Next())
	{
		WmiClassObject classObj(wbemClassObj);

		Log(L"BuildNumber", classObj.String(L"BuildNumber"));
		Log(L"BuildType", classObj.String(L"BuildType"));
		Log(L"Locale", classObj.String(L"Locale"));
		Log(L"Manufacturer", classObj.String(L"Manufacturer"));
		Log(L"Name", classObj.String(L"Name"));
		Log(L"OSType", classObj.Int32(L"OSType"));
		Log(L"Primary", classObj.Bool(L"Primary"));
		Log(L"BootDevice", classObj.String(L"BootDevice"));
		Log(L"CountryCode", classObj.String(L"CountryCode"));
		Log(L"Version", classObj.String(L"Version"));
		Log(L"NumberOfUsers", classObj.Int32(L"NumberOfUsers"));
		Log(L"CurrentTimeZone", classObj.Short(L"CurrentTimeZone"));
		Log(L"LastBootUpTime", classObj.String(L"LastBootUpTime"));
		Log(L"InstallDate", classObj.String(L"InstallDate"));
		Log(L"OSArchitecture", classObj.String(L"OSArchitecture"));
		Log(L"OperatingSystemSKU", classObj.Int32(L"OperatingSystemSKU"));
		Log(L"OSLanguage", classObj.Int32(L"OSLanguage"));

	}

	return 0;
}