#include "Header.hpp"

int MSStorageDriverFailurePredictStatus()
{
	//\\MONARCH\ROOT\WMI:MSStorageDriver_FailurePredictStatus.InstanceName="SCSI\\Disk&Ven_&Prod_ST3000NC000-1CX1\\4&3449ecb1&0&040000_0"
	WmiProxy wmi;
	WmiServer rootCim2 = wmi.ConnectServer(L"ROOT\\WMI");

	std::wstring query = L"select * from MSStorageDriver_FailurePredictStatus where InstanceName = \"SCSI\\\\Disk&Ven_Samsung&Prod_SSD_860_PRO_1TB\\\\4&dc154c5&0&000000_0\"";
	// "SCSI\\Disk&Ven_Samsung&Prod_SSD_860_PRO_1TB\\4&dc154c5&0&000000_0"
	//WmiObjectEnumerator enumerator = rootCim2.Query(L"SELECT * FROM MSStorageDriver_FailurePredictStatus");
	WmiObjectEnumerator enumerator = rootCim2.Query(query);

	while (IWbemClassObject* ptrClsObj = enumerator.Next())
	{
		WmiClassObject classObj(ptrClsObj);
		Log(L"PredictFailure", classObj.Bool(L"PredictFailure"));
		Log(L"Active", classObj.Bool(L"Active"));
		Log(L"Reason", classObj.Int32(L"Reason"));
		Log(L"InstanceName", classObj.String(L"InstanceName"));
	}

	return 0;
}