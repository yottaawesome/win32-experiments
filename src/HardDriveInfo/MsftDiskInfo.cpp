#pragma once
#include "Header.hpp"
#include <iostream>
#include <WTypes.h>
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

int MsftDiskInfo()
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
		IID_IWbemLocator,
		(LPVOID*)&pLoc);

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
		_bstr_t(L"ROOT\\MICROSOFT\\WINDOWS\\STORAGE"), // Object path of WMI namespace
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

	std::wcout << "Connected to ROOT\\MICROSOFT\\WINDOWS\\STORAGE namespace" << std::endl;

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

	// For example, get the name of the operating system
	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM MSFT_PhysicalDisk"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		std::wcout << "Could not query Disk Spindle. Error code = 0x"
			<< std::hex << hres << std::endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	IWbemClassObject* pclsObj;
	ULONG uReturn = 0;
	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
		if (0 == uReturn)
		{
			break;
		}
		VARIANT vtProp;

		// Get the value of the Name property
		hr = pclsObj->Get(L"FriendlyName", 0, &vtProp, 0, 0);
		std::wcout << " FriendlyName : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"MediaType", 0, &vtProp, 0, 0);
		std::wcout << " MediaType : " << vtProp.intVal << std::endl;

		hr = pclsObj->Get(L"UniqueIdFormat", 0, &vtProp, 0, 0);
		std::wcout << " UniqueIdFormat : " << vtProp.intVal << std::endl;

		hr = pclsObj->Get(L"DeviceId", 0, &vtProp, 0, 0);
		std::wcout << " DeviceId : " << vtProp.bstrVal << std::endl;

		//hr = pclsObj->Get(L"Description", 0, &vtProp, 0, 0);
		//std::wcout << " Description : " << vtProp.bstrVal << std::endl;

		//hr = pclsObj->Get(L"PartNumber", 0, &vtProp, 0, 0);
		//std::wcout << " PartNumber : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"FirmwareVersion", 0, &vtProp, 0, 0);
		std::wcout << " FirmwareVersion : " << vtProp.bstrVal << std::endl;

		//hr = pclsObj->Get(L"SoftwareVersion", 0, &vtProp, 0, 0);
		//std::wcout << " SoftwareVersion : " << vtProp.bstrVal << std::endl;

		hr = pclsObj->Get(L"HealthStatus", 0, &vtProp, 0, 0);
		std::wcout << " HealthStatus : " << vtProp.intVal << std::endl;

		VariantClear(&vtProp);

		pclsObj->Release();
	}

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	//pclsObj->Release();
	CoUninitialize();

	return 0;
}