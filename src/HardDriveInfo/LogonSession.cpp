#include "Header.hpp"
#include "iostream"

int Win32LogonSession()
{
	WmiProxy wmiProxy;
	WmiServer server = wmiProxy.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator objectEnum = server.Query(L"select * from Win32_LogonSession");

	while (IWbemClassObject* wbemClassObj = objectEnum.Next())
	{
		WmiClassObject classObj(wbemClassObj);

		// WMI datetimes are in UTC format
		// https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-systemtimetotzspecificlocaltime
		// https://docs.microsoft.com/en-us/windows/win32/wmisdk/date-and-time-format
		// https://docs.microsoft.com/en-us/previous-versions/tn-archive/ee198928(v=technet.10)?redirectedfrom=MSDN
		// 20200310075344.264044+660
		// YYYYMMDDHHMMSS.xxxxxx±UUU
		// (year)(month)(day)(hour in 24 hr format)(minutes)(seconds).(milliseconds)(minutes ahead or behind Greenwich Mean Time (GMT))
		Log(L"StartTime", classObj.String(L"StartTime"));
		Log(L"Name", classObj.String(L"Name"));
		Log(L"Status", classObj.String(L"Status"));
		Log(L"LogonId", classObj.String(L"LogonId"));
		Log(L"Status", classObj.String(L"Status"));
		Log(L"AuthenticationPackage", classObj.String(L"AuthenticationPackage"));
		Log(L"Caption", classObj.String(L"Caption"));
		Log(L"Description", classObj.String(L"Description"));
	}

	return 0;
}
