#include "Header.hpp"
#include <istream>
#include <sstream>
#include <functional>

using ErrorCallback = std::function<std::string()>;

void CheckHr(HRESULT hr, const std::string& msg)
{
	if (FAILED(hr))
	{
		std::stringstream ss;
		_com_error ce(hr);
		ss
			<< msg
			<< std::endl
			<< " Error code: 0x" << std::hex << hr
			<< std::endl
			<< " COM error message: " << ce.ErrorMessage()
			<< std::endl;
		throw std::runtime_error(ss.str());
	}
}

void CheckHr(HRESULT hr, ErrorCallback lambda)
{
	if (FAILED(hr))
	{
		std::stringstream ss;
		_com_error ce(hr);
		ss
			<< lambda()
			<< std::endl
			<< " Error code: 0x" << std::hex << hr
			<< std::endl
			<< " COM error message: " << ce.ErrorMessage()
			<< std::endl;
		throw std::runtime_error(ss.str());
	}
}

ComInitialiser::ComInitialiser(COINIT apartmentThreadingMode)
{
	// Initialise COM
	HRESULT hr = CoInitializeEx(nullptr, apartmentThreadingMode);
	CheckHr(hr, "Failed to initialise COM.y.");

	// Set general COM security levels. This can only be set once.
	HRESULT hres = CoInitializeSecurity(
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
	CheckHr(hres, "Failed to initialize COM security.");
}

ComInitialiser::~ComInitialiser()
{
	CoUninitialize();
}
