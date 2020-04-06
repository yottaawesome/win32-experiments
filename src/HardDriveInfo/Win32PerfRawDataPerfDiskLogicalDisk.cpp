#include "Header.hpp"

int Win32PerfRawDataPerfDiskLogicalDisk()
{
	WmiProxy wmiProxy;
	WmiServer server = wmiProxy.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator objectEnum = server.Query(L"select * from Win32_PerfRawData_PerfDisk_LogicalDisk");

	while (IWbemClassObject* wbemClassObj = objectEnum.Next())
	{
		WmiClassObject classObj(wbemClassObj);

		Log(L"Name", classObj.String(L"Name"));
		Log(L"FreeMegabytes", classObj.Int32(L"FreeMegabytes"));
		Log(L"PercentIdleTime", classObj.StringAsUInt64(L"PercentIdleTime"));
		Log(L"PercentDiskWriteTime", classObj.StringAsUInt64(L"PercentDiskWriteTime"));
		Log(L"PercentDiskReadTime", classObj.StringAsUInt64(L"PercentDiskReadTime"));
		Log(L"CurrentDiskQueueLength", classObj.Int32(L"CurrentDiskQueueLength"));
	}

	return 0;
}