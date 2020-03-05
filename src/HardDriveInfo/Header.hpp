#pragma once
#include <string>
#include <memory>
#include <comdef.h>
#include <Wbemidl.h>
#include <wrl/client.h>

int MsftDiskInfo();
int Win32DiskInfo();
int Win32LogicalDisk();
int Win32RamInfo();
int Win32RamInfo2();
int Win32ProcessorInfo();

class WmiClassObject
{
	public:
		WmiClassObject(IWbemClassObject* clsObj);

		[[nodiscard]] const short Short(const std::wstring& name);
		[[nodiscard]] const int Int32(const std::wstring& name);
		[[nodiscard]] const long Long(const std::wstring& name);
		[[nodiscard]] const long long Int64(const std::wstring& name);

		[[nodiscard]] const unsigned short UShort(const std::wstring& name);
		[[nodiscard]] const unsigned int UInt32(const std::wstring& name);
		[[nodiscard]] const unsigned long ULong(const std::wstring& name);
		[[nodiscard]] const unsigned long long UInt64(const std::wstring& name);

		[[nodiscard]] const std::wstring String(const std::wstring& name);
		[[nodiscard]] const std::wstring StringOrEmpty(const std::wstring& name);

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

class ComInitialiser final
{
	public:
		ComInitialiser(COINIT aparthmentThreadingMode = COINIT_MULTITHREADED);
		~ComInitialiser();
};
