#include "Header.hpp"
#include <iostream>
#include <WTypes.h>
#include <iostream>
#include <sstream>
#include <comdef.h>
#include <Wbemidl.h>
#include <stdexcept>

WmiProxy::WmiProxy()
	: m_wbemLocator(nullptr)
{
	// We assume the process has already initialised COM, so we won't worry about that.

	// Obtain WMI Service locator
	HRESULT hr = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator,
		(LPVOID*)&m_wbemLocator);
	Util::CheckHr(hr, "Failed to create IWbemLocator proxy.");
}

WmiProxy::~WmiProxy()
{
	if (m_wbemLocator)
		m_wbemLocator->Release();
}

WmiServer WmiProxy::ConnectServer(const std::wstring& server)
{
	IWbemServices* wbemServicesProxy;

	HRESULT hres;
	// Connect to the requested namespace with
	// the current user and obtain the service proxy
	// to make IWbemServices calls.
	hres = m_wbemLocator->ConnectServer(
		_bstr_t(server.c_str()),	// Object path of WMI namespace
		nullptr,                    // User m_name. NULL = current user
		nullptr,                    // User password. NULL = current
		0,							// Locale. NULL indicates current
		0,							// Security flags.
		0,							// Authority (e.g. Kerberos)
		0,							// Context object 
		&wbemServicesProxy			// pointer to IWbemServices proxy
	);

	static std::string convertedNamespace = Util::ConvertWStringToString(server);
	static std::string errorMsg = "Could not connect to requested namespace: " + convertedNamespace;
	Util::CheckHr(hres, errorMsg);

	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------
	hres = CoSetProxyBlanket(
		wbemServicesProxy,				// Indicates the proxy to set
		RPC_C_AUTHN_WINNT,				// RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,				// RPC_C_AUTHZ_xxx
		nullptr,                        // Server principal m_name 
		RPC_C_AUTHN_LEVEL_CALL,			// RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE,	// RPC_C_IMP_LEVEL_xxx
		nullptr,                        // client identity
		EOAC_NONE						// proxy capabilities 
	);

	Util::CheckHr(hres, "Failed to set proxy blanket.");

	return WmiServer(wbemServicesProxy);
}
