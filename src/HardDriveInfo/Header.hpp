#pragma once
#include <string>
#include <memory>
#include <comdef.h>
#include <Wbemidl.h>
#include <wrl/client.h>

int MsftDiskInfo();
int Win32DiskInfo();
int Win32RamInfo();
int Win32RamInfo2();

class WmiServerConnection
{
	public:
		WmiServerConnection(IWbemServices* pSvc);

		[[nodiscard]]
		IEnumWbemClassObject* Query(const std::wstring& string);

	private:
		Microsoft::WRL::ComPtr<IWbemServices> w_wbemService;
};

class Wmi
{
	public:
		Wmi();

		[[nodiscard]]
		WmiServerConnection ConnectServer(const std::wstring& server);

	private:
		Microsoft::WRL::ComPtr<IWbemLocator> m_wbemLocator;
};
