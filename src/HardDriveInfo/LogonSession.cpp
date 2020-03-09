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
		auto result = classObj.String(L"StartTime");

		std::wcout << result << std::endl;
	}

	return 0;
}
