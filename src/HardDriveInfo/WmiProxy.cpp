#include "Header.hpp"
#include <iostream>
#include <WTypes.h>
#include <iostream>
#include <sstream>
#include <comdef.h>
#include <Wbemidl.h>
#include <stdexcept>

WmiProxy::WmiProxy()
{
	HRESULT hres;

	// Step 1: --------------------------------------------------
	// Initialize COM. ------------------------------------------

	//hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	//if (FAILED(hres))
	//{
	//	std::stringstream ss;
	//	ss << L"Failed to initialize COM library. Error code = 0x"
	//		<< std::hex << hres
	//		<< std::endl;
	//	throw std::runtime_error(ss.str());
	//}

	// Step 2: Set general COM security levels 

	hres = CoInitializeSecurity(
		nullptr,
		-1,								// COM authentication
		nullptr,                        // Authentication services
		nullptr,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,		// Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE,	// Default Impersonation  
		nullptr,                        // Authentication info
		EOAC_NONE,						// Additional capabilities 
		nullptr                         // Reserved
	);

	if (FAILED(hres))
	{
		std::stringstream ss;
		ss
			<< L"Failed to initialize COM security. Error code = 0x"
			<< std::hex << hres
			<< std::endl;
		throw std::runtime_error(ss.str());
	}

	// Step 3: Obtain the initial locator to WMI -------------------------

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator,
		(LPVOID*)&m_wbemLocator);

	if (FAILED(hres))
	{
		std::stringstream ss;
		ss
			<< L"Failed to create IWbemLocator object."
			<< " Err code = 0x"
			<< std::hex << hres
			<< std::endl;
		throw std::runtime_error(ss.str());
		CoUninitialize();
	}
}

WmiServer WmiProxy::ConnectServer(const std::wstring& server)
{
	IWbemServices* pSvc;

	HRESULT hres;
	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = m_wbemLocator->ConnectServer(
		_bstr_t(server.c_str()),	// Object path of WMI namespace
		nullptr,                    // User name. NULL = current user
		nullptr,                    // User password. NULL = current
		0,							// Locale. NULL indicates current
		0,							// Security flags.
		0,							// Authority (e.g. Kerberos)
		0,							// Context object 
		&pSvc						// pointer to IWbemServices proxy
	);

	if (FAILED(hres))
	{
		std::stringstream ss;
		ss
			<< "Could not connect. Error code = 0x"
			<< std::hex << hres
			<< std::endl;
		throw std::runtime_error(ss.str());
	}

	std::wcout << "Connected to ROOT\\CIMV2 WMI namespace" << std::endl;

	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------
	hres = CoSetProxyBlanket(
		pSvc,							// Indicates the proxy to set
		RPC_C_AUTHN_WINNT,				// RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,				// RPC_C_AUTHZ_xxx
		nullptr,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,			// RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE,	// RPC_C_IMP_LEVEL_xxx
		nullptr,                        // client identity
		EOAC_NONE						// proxy capabilities 
	);

	if (FAILED(hres))
	{
		std::stringstream ss;
		ss
			<< "Could not set proxy blanket. Error code = 0x"
			<< std::hex << hres
			<< std::endl;
	}

	return WmiServer(pSvc);
}
