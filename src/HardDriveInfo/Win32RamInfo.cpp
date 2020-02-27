#include "Header.hpp"
#include <iostream>
#include <WTypes.h>
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

BSTR GetValue(BSTR value)
{
	if(value == nullptr)
		return BSTR(L"(Unspecified)");
	return value;
}

int Win32RamInfo()
{
	HRESULT hres;

	// Step 1: --------------------------------------------------
	// Initialize COM. ------------------------------------------

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
		std::wcout << L"Failed to initialize COM library. Error code = 0x"
			<< std::hex << hres << std::endl;
		return 1;                  // Program has failed.
	}

	// Step 2: --------------------------------------------------
	// Set general COM security levels --------------------------
	// Note: If you are using Windows 2000, you need to specify -
	// the default authentication credentials for a user by using
	// a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
	// parameter of CoInitializeSecurity ------------------------

	hres = CoInitializeSecurity(
		NULL,
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
	);

	if (FAILED(hres))
	{
		std::wcout << L"Failed to initialize security. Error code = 0x"
			<< std::hex << hres << std::endl;
		CoUninitialize();
		return 1;                    // Program has failed.
	}

	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------

	IWbemLocator* pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)&pLoc);

	if (FAILED(hres))
	{
		std::wcout << L"Failed to create IWbemLocator object."
			<< " Err code = 0x"
			<< std::hex << hres << std::endl;
		CoUninitialize();
		return 1;                 // Program has failed.
	}

	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method

	IWbemServices* pSvc = NULL;

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (e.g. Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
	);

	if (FAILED(hres))
	{
		std::wcout << "Could not connect. Error code = 0x"
			<< std::hex << hres << std::endl;
		pLoc->Release();
		CoUninitialize();
		return 1;                // Program has failed.
	}

	std::wcout << "Connected to ROOT\\CIMV2 WMI namespace" << std::endl;


	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------
	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
	);

	if (FAILED(hres))
	{
		std::wcout << "Could not set proxy blanket. Error code = 0x"
			<< std::hex << hres << std::endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_PhysicalMemory"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	IWbemClassObject* pclsObj;
	ULONG uReturn = 0;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);

		if (0 == uReturn)
			break;

		VARIANT vtProp;
		hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
		std::wcout << L" Manufacturer : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"Model", 0, &vtProp, 0, 0);
		std::wcout << L" Model : " << GetValue(vtProp.bstrVal) << std::endl;

		hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
		std::wcout << L" Name : " << GetValue(vtProp.bstrVal) << std::endl;

		hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		std::wcout << L" SerialNumber : " << GetValue(vtProp.bstrVal) << std::endl;
		std::wcout << L" type : " << vtProp.vt << std::endl;

		hr = pclsObj->Get(L"SKU", 0, &vtProp, 0, 0);
		std::wcout << L" SKU : " << GetValue(vtProp.bstrVal) << std::endl;

		hr = pclsObj->Get(L"SMBIOSMemoryType", 0, &vtProp, 0, 0);
		std::wcout << L" SMBIOSMemoryType : " << vtProp.uintVal << std::endl;

		hr = pclsObj->Get(L"BankLabel", 0, &vtProp, 0, 0);
		std::wcout << " BankLabel : " << GetValue(vtProp.bstrVal) << std::endl;

		hr = pclsObj->Get(L"TypeDetail", 0, &vtProp, 0, 0);
		std::wcout << L" TypeDetail : " << vtProp.uintVal << std::endl;

		hr = pclsObj->Get(L"Status", 0, &vtProp, 0, 0);
		std::wcout << L" Status : " << GetValue(vtProp.bstrVal) << std::endl;
		std::wcout << L" type : " << vtProp.vt << std::endl;

		hr = pclsObj->Get(L"MemoryType", 0, &vtProp, 0, 0);
		std::wcout << L" MemoryType : " << vtProp.uintVal << std::endl;

		hr = pclsObj->Get(L"PositionInRow", 0, &vtProp, 0, 0);
		std::wcout << L" PositionInRow : " << vtProp.uintVal << std::endl;

		VariantClear(&vtProp);
		hr = pclsObj->Get(L"Capacity", 0, &vtProp, 0, 0);
		if(FAILED(hr))
			std::wcout << L"Could not determine capacity" << std::endl;
		std::wcout << L" Capacity : " << vtProp.bstrVal << std::endl;
		std::wcout << L" type : " << vtProp.vt << std::endl;

		//Get<vtProp.vt>();

		hr = pclsObj->Get(L"Speed", 0, &vtProp, 0, 0);
		std::wcout << L" Speed : " << vtProp.uintVal << std::endl;

		hr = pclsObj->Get(L"TotalWidth", 0, &vtProp, 0, 0);
		std::wcout << L" TotalWidth : " << vtProp.uintVal << std::endl;

		hr = pclsObj->Get(L"DataWidth", 0, &vtProp, 0, 0);
		std::wcout << L" DataWidth : " << vtProp.uintVal << std::endl;

		hr = pclsObj->Get(L"Description", 0, &vtProp, 0, 0);
		std::wcout << L" Description : " << GetValue(vtProp.bstrVal) << std::endl;

		VariantClear(&vtProp);
		pclsObj->Release();

		std::wcout << std::endl << std::endl;
	}

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	CoUninitialize();

	ULONGLONG TotalMemoryInKilobytes;
	GetPhysicallyInstalledSystemMemory(
		&TotalMemoryInKilobytes
	);
	std::wcout << " GetPhysicallyInstalledSystemMemory : " << TotalMemoryInKilobytes << std::endl;

	MEMORYSTATUSEX lpBuffer{};
	lpBuffer.dwLength = sizeof(lpBuffer);
	GlobalMemoryStatusEx(&lpBuffer);
	std::wcout << " MEMORYSTATUSEX.dwMemoryLoad : " << lpBuffer.dwMemoryLoad << std::endl;
	std::wcout << " MEMORYSTATUSEX.ullAvailPhys : " << lpBuffer.ullAvailPhys << std::endl;
	std::wcout << " MEMORYSTATUSEX.ullTotalPhys : " << lpBuffer.ullTotalPhys << std::endl;
	std::wcout << " MEMORYSTATUSEX.ullAvailVirtual : " << lpBuffer.ullAvailVirtual << std::endl;

	return 0;
}

int Win32RamInfo2()
{
	using Microsoft::WRL::ComPtr;

	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	Wmi wmi;
	WmiServerConnection rootCim2 = wmi.ConnectServer(L"ROOT\\CIMV2");
	ComPtr<IEnumWbemClassObject> enumerator = rootCim2.Query(L"SELECT * FROM Win32_PhysicalMemory");

	ULONG uReturn = 0;
	ComPtr<IWbemClassObject> pclsObj;

	while (1)
	{
		HRESULT hr = enumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
		if (uReturn == 0)
		{
			break;
		}

		VARIANT vtProp;
		hr = pclsObj->Get(L"Capacity", 0, &vtProp, 0, 0);
		std::wcout << L" Capacity : " << vtProp.bstrVal << std::endl;
	}

	// Don't call CoUninitalize before the ComPtrs have been released
	// It's best to manage this through RAII
	//CoUninitialize();

	return 0;
}
