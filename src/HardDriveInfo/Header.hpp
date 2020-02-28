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
int Win32ProcessorInfo();

class WbemClassObject
{
	public:
		WbemClassObject(IWbemClassObject* clsObj);

		const short Short(const std::wstring& name);
		const int Int32(const std::wstring& name);
		const long Long(const std::wstring& name);
		const long long Int64(const std::wstring& name);

		const unsigned short UnsignedShort(const std::wstring& name);
		const unsigned int UnsignedInt32(const std::wstring& name);
		const unsigned long UnsignedLong(const std::wstring& name);
		const unsigned long long UnsignedInt64(const std::wstring& name);

		const std::wstring String(const std::wstring& name);

	private:
		Microsoft::WRL::ComPtr<IWbemClassObject> m_clsObj;
};

class WmiObjectEnumerator
{
	public:
		WmiObjectEnumerator(IEnumWbemClassObject* wbemEnum);
		IWbemClassObject* Next();

	private:
		Microsoft::WRL::ComPtr<IEnumWbemClassObject> m_wbemEnum;
};

class WmiServer
{
	public:
		WmiServer(IWbemServices* pSvc);

		[[nodiscard]]
		WmiObjectEnumerator Query(const std::wstring& string);

	private:
		Microsoft::WRL::ComPtr<IWbemServices> w_wbemService;
};

class WmiProxy
{
	public:
		WmiProxy();

		[[nodiscard]]
		WmiServer ConnectServer(const std::wstring& server);

	private:
		Microsoft::WRL::ComPtr<IWbemLocator> m_wbemLocator;
};
