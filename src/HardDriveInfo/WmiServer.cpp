#include "Header.hpp"
#include <stdexcept>

WmiServer::WmiServer(IWbemServices* pSvc)
	: w_wbemService(pSvc)
{
	if (pSvc == nullptr)
		throw std::runtime_error("WmiServer cannot be initialised with a null locator");
}

WmiObjectEnumerator WmiServer::Query(const std::wstring& string)
{
	IEnumWbemClassObject* pEnumerator = nullptr;
	HRESULT hres = w_wbemService->ExecQuery(
		bstr_t("WQL"),
		bstr_t(string.c_str()),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		nullptr,
		&pEnumerator);

	if (FAILED(hres))
		throw std::runtime_error("Query failed");

	return WmiObjectEnumerator(pEnumerator);
}
