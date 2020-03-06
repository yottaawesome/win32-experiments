#include "Header.hpp"
#include <iostream>
#include <map>

int Win32ProcessorInfo()
{
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
		std::wcout << L"\t SystemName : "
			<< classObj.String(L"SystemName")
			<< std::endl;
		std::wcout << L"\t UniqueId : "
			<< classObj.String(L"UniqueId")
			<< std::endl;
		std::wcout << L"\t SocketDesignation : "
			<< classObj.String(L"SocketDesignation")
			<< std::endl;
		std::wcout << L"\t ProcessorId : "
			<< classObj.String(L"ProcessorId")
			<< std::endl;

		unsigned short archVal = classObj.UShort(L"Architecture");
		std::map<unsigned short, std::wstring> arch =
		{
			{0,L"x86"},
			{1,L"MIPS"},
			{2,L"Alpha"},
			{3,L"PowerPC"},
			{5,L"ARM"},
			{6,L"ia64"},
			{9,L"x64"},
		};
		std::wcout << L"\t Architecture : "
			<< arch[archVal]
			<< std::endl;
	}

	return 0;
}