#include "Header.hpp"
#include <stdexcept>

WmiServer::WmiServer(IWbemServices* wbemService)
	: m_wbemService(wbemService)
{
	if (wbemService == nullptr)
		throw std::runtime_error("WmiServerConnection cannot be initialised with a null locator");
}

WmiServer::~WmiServer()
{
	m_wbemService->Release();
}

WmiObjectEnumerator WmiServer::Query(const std::wstring& string)
{
	IEnumWbemClassObject* enumerator = nullptr;
	HRESULT hres = m_wbemService->ExecQuery(
		bstr_t(L"WQL"),
		bstr_t(string.c_str()),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		nullptr,
		&enumerator);

	Util::CheckHr(hres, [&string]() -> std::string { return "Query failed: " + Util::ConvertWStringToString(string); });

	return WmiObjectEnumerator(enumerator);
}

WmiClassObject WmiServer::GetClassObject(const wchar_t* objectPath)
{
	IWbemClassObject* ppObject = nullptr;
	IWbemCallResult* ppCallResult = nullptr;
	//HRESULT hr = m_wbemService->GetObjectW(_bstr_t(L"Win32_DiskDrive"), WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &ppObject, &ppCallResult);
	HRESULT hr = m_wbemService->GetObjectW(_bstr_t(objectPath), WBEM_FLAG_RETURN_WBEM_COMPLETE, nullptr, &ppObject, nullptr);
	if (hr != WBEM_S_NO_ERROR)
		throw std::runtime_error("Failed");

	return WmiClassObject(ppObject);
}
