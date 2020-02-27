#include "Header.hpp"
#include <iostream>
#include <WTypes.h>
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <stdexcept>

Wmi::Wmi()
{
	HRESULT hres;

	// Step 1: --------------------------------------------------
	// Initialize COM. ------------------------------------------

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
		std::wcout
			<< L"Failed to initialize COM library. Error code = 0x"
			<< std::hex << hres
			<< std::endl;
		throw std::runtime_error("");
	}
}

Wmi::~Wmi()
{

}

void Wmi::ConnectServer(std::wstring server)
{

}
