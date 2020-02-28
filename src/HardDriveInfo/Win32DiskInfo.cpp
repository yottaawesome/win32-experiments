#include "Header.hpp"
#include <iostream>
#include <WTypes.h>
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

int Win32DiskInfo()
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

	// Step 6: --------------------------------------------------
	// Use the IWbemServices pointer to make requests of WMI ----

	/*
	* Disk Drive information
	*/

	// For example, get the name of the operating system
	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_DiskPartition"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		std::wcout << "Query for operating system name failed."
			<< " Error code = 0x"
			<< std::hex << hres << std::endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	// Step 7: -------------------------------------------------
	// Get the data from the query in step 6 -------------------

	IWbemClassObject* pclsObj;
	ULONG uReturn = 0;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);

		if (0 == uReturn)
		{
			break;
		}

		VARIANT vtProp;
		VariantInit(&vtProp);

		// Get the value of the Name property
		hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
		std::wcout << " Name : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"StartingOffset", 0, &vtProp, 0, 0);
		std::wcout << " Starting Offset : " << vtProp.uintVal << std::endl;
		VariantClear(&vtProp);
		pclsObj->Release();
	}


	/*
	* Disk Drive information
	*/

	pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_DiskDrive "),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);
	if (FAILED(hres))
	{
		std::wcout << "Query for operating system name failed."
			<< " Error code = 0x"
			<< std::hex << hres << std::endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	// Step 7: -------------------------------------------------
	// Get the data from the query in step 6 -------------------

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);
		if (0 == uReturn)
		{
			break;
		}
		VARIANT vtProp;
		// Get the value of the Name property
		hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
		std::wcout << " Name : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
		std::wcout << " Manufacturer : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"Model", 0, &vtProp, 0, 0);
		std::wcout << " Model : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"Signature", 0, &vtProp, 0, 0);
		std::wcout << " Signature : " << vtProp.uintVal << std::endl;

		hr = pclsObj->Get(L"DeviceID", 0, &vtProp, 0, 0);
		std::wcout << " DeviceId : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"Caption", 0, &vtProp, 0, 0);
		std::wcout << " Caption : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"MediaType", 0, &vtProp, 0, 0);
		std::wcout << " Media Type : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"FirmwareRevision", 0, &vtProp, 0, 0);
		std::wcout << " FirwareRevision : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"InterfaceType", 0, &vtProp, 0, 0);
		std::wcout << " InterfaceType : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"Description", 0, &vtProp, 0, 0);
		std::wcout << " Description : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		std::wcout << " SerialNumber : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"SystemName", 0, &vtProp, 0, 0);
		std::wcout << " SystemName : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"TotalCylinders", 0, &vtProp, 0, 0);
		std::wcout << " TotalCylinders : " << vtProp.llVal << std::endl;

		hr = pclsObj->Get(L"TotalHeads", 0, &vtProp, 0, 0);
		std::wcout << " TotalHeads : " << vtProp.uintVal << std::endl;

		hr = pclsObj->Get(L"TotalTracks", 0, &vtProp, 0, 0);
		std::wcout << " TotalTracks : " << vtProp.llVal << std::endl;

		hr = pclsObj->Get(L"Status", 0, &vtProp, 0, 0);
		std::wcout << " Status : " << vtProp.bstrVal << std::endl;

		VariantClear(&vtProp);

		pclsObj->Release();
	}



	// Cleanup
	// ========

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	CoUninitialize();

	return 0;
}
