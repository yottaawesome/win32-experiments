#include "Header.hpp"
#include <iostream>

int Win32ProcessorInfo()
{
	using Microsoft::WRL::ComPtr;

	WmiProxy wmiProxy;
	WmiServer server = wmiProxy.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator objectEnum = server.Query(L"select * from Win32_Processor");

	ComPtr<IWbemClassObject> classObj = nullptr;
	while (classObj = objectEnum.Next())
	{
		WbemClassObject wbemClassObj(classObj.Get());

		std::wcout << L"\t Manufacturer : " << wbemClassObj.String(L"Manufacturer") << std::endl;

		/*hr = classObj->Get(L"Name", 0, &vtProp, 0, 0);
		std::wcout << L"\t Name : " << vtProp.bstrVal << std::endl;

		hr = classObj->Get(L"LoadPercentage", 0, &vtProp, 0, 0);
		std::wcout << L"\t LoadPercentage : " << vtProp.uiVal << std::endl;

		hr = classObj->Get(L"MaxClockSpeed", 0, &vtProp, 0, 0);
		std::wcout << L"\t MaxClockSpeed : " << vtProp.uintVal << std::endl;

		hr = classObj->Get(L"NumberOfCores", 0, &vtProp, 0, 0);
		std::wcout << L"\t NumberOfCores : " << vtProp.uintVal << std::endl;

		hr = classObj->Get(L"NumberOfEnabledCore", 0, &vtProp, 0, 0);
		std::wcout << L"\t NumberOfEnabledCore : " << vtProp.uintVal << std::endl;

		hr = classObj->Get(L"NumberOfLogicalProcessors", 0, &vtProp, 0, 0);
		std::wcout << L"\t NumberOfLogicalProcessors : " << vtProp.uintVal << std::endl;

		hr = classObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		std::wcout << L"\t SerialNumber : " << vtProp.bstrVal << std::endl;

		hr = classObj->Get(L"DataWidth", 0, &vtProp, 0, 0);
		std::wcout << L"\t DataWidth : " << vtProp.uiVal << std::endl;*/
	}

	return 0;
}