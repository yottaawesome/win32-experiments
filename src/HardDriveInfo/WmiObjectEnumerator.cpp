#include "Header.hpp"
#include <stdexcept>

WmiObjectEnumerator::WmiObjectEnumerator(IEnumWbemClassObject* wbemEnum)
	: m_wbemEnum(wbemEnum)
{
	if (wbemEnum == nullptr)
		throw std::runtime_error("WmiObjectEnumerator cannot be initialised with a null IEnumWbemClassObject.");
}

IWbemClassObject* WmiObjectEnumerator::Next()
{
	IWbemClassObject* object;
	ULONG objectsReturned = 0;
	HRESULT hr = m_wbemEnum->Next(WBEM_INFINITE, 1, &object, &objectsReturned);

	return objectsReturned > 0 ? object : nullptr;
}