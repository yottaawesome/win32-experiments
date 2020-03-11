#include "Header.hpp"

int Win32PerfRawDataPerfProcProcess()
{
	WmiProxy wmiProxy;
	WmiServer server = wmiProxy.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator objectEnum = server.Query(L"select * from Win32_PerfRawData_PerfProc_Process");

	while (IWbemClassObject* wbemClassObj = objectEnum.Next())
	{
		WmiClassObject classObj(wbemClassObj);

		Log(L"Name", classObj.String(L"Name"));
		Log(L"PrivateBytes", classObj.StringAsUInt64(L"PrivateBytes"));
		Log(L"PercentProcessorTime", classObj.StringAsUInt64(L"PercentProcessorTime"));
		Log(L"PercentPrivilegedTime", classObj.StringAsUInt64(L"PercentPrivilegedTime"));
		Log(L"PercentUserTime", classObj.StringAsUInt64(L"PercentUserTime"));
		Log(L"======================", L"======================");
	}

	return 0;
}
