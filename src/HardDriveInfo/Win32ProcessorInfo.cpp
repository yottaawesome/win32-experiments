#include "Header.hpp"
#include <iostream>

int Win32ProcessorInfo()
{
	using Microsoft::WRL::ComPtr;

	WmiProxy wmiProxy;
	WmiServer server = wmiProxy.ConnectServer(L"ROOT\\CIMV2");
	WmiObjectEnumerator objectEnum = server.Query(L"select * from Win32_Processor");

	while (IWbemClassObject* wbemClassObj = objectEnum.Next())
	{
		WmiClassObject classObj(wbemClassObj);

		std::wcout << L"\t Manufacturer : "
			<< classObj.String(L"Manufacturer")
			<< std::endl;
		std::wcout << L"\t Name : "
			<< classObj.String(L"Name")
			<< std::endl;
		std::wcout << L"\t SerialNumber : "
			<< classObj.String(L"SerialNumber")
			<< std::endl;
		std::wcout << L"\t LoadPercentage : "
			<< classObj.UShort(L"LoadPercentage")
			<< std::endl;
		std::wcout << L"\t MaxClockSpeed : "
			<< classObj.UInt32(L"MaxClockSpeed")
			<< std::endl;
		std::wcout << L"\t NumberOfCores : "
			<< classObj.UInt32(L"NumberOfCores")
			<< std::endl;
		std::wcout << L"\t NumberOfEnabledCore : " 
			<< classObj.UInt32(L"NumberOfEnabledCore")
			<< std::endl;
		std::wcout << L"\t NumberOfLogicalProcessors : "
			<< classObj.UInt32(L"NumberOfLogicalProcessors")
			<< std::endl;
		std::wcout << L"\t DataWidth : "
			<< classObj.UShort(L"DataWidth")
			<< std::endl;
	}

	return 0;
}