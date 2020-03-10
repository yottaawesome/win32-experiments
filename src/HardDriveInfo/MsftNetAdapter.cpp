#include "Header.hpp"

int MsftNetworkAdapter()
{
	// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/hh968170(v=vs.85)?redirectedfrom=MSDN
	WmiProxy wmiProxy;
	WmiServer server = wmiProxy.ConnectServer(L"Root\\StandardCimv2");
	WmiObjectEnumerator objectEnum = server.Query(L"select * from MSFT_NetAdapter");

	while (IWbemClassObject* wbemClassObj = objectEnum.Next())
	{
		WmiClassObject classObj(wbemClassObj);

		Log(L"Name", classObj.String(L"Name"));
		Log(L"DeviceName", classObj.String(L"DeviceName"));
		Log(L"InterfaceName", classObj.String(L"InterfaceName"));
		Log(L"InterfaceDescription", classObj.String(L"InterfaceDescription"));
		Log(L"Status", classObj.String(L"Status"));
		Log(L"DeviceID", classObj.String(L"DeviceID"));
		Log(L"PortNumber", classObj.Int32(L"PortNumber"));
		Log(L"InterfaceGuid", classObj.String(L"InterfaceGuid"));
		Log(L"Virtual", classObj.Bool(L"Virtual"));
		Log(L"InterfaceOperationalStatus", classObj.Int32(L"InterfaceOperationalStatus"));
		Log(L"MediaConnectState", classObj.Int32(L"MediaConnectState"));
		//Log(L"LinkTechnology", classObj.Short(L"LinkTechnology"));
		Log(L"InterfaceType", classObj.Int32(L"InterfaceType"));
		Log(L"NdisPhysicalMedium", classObj.Int32(L"NdisPhysicalMedium"));

		const std::vector<std::wstring> networkAddresses = classObj.StringVector(L"NetworkAddresses");
		for (const std::wstring& str : networkAddresses)
		{
			Log(L"NetworkAddress", str);
		}
		Log(L"===============", L"===============");

	}

	return 0;
}
