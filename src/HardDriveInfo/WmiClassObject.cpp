#include "Header.hpp"
#include <iostream>

WmiClassObject::WmiClassObject(IWbemClassObject* clsObj)
	: m_clsObj(clsObj)
{ }

const short WmiClassObject::Short(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.iVal;
}

const int WmiClassObject::Int32(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.intVal;
}

const long WmiClassObject::Long(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.lVal;
}

const long long WmiClassObject::Int64(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.llVal;
}

const unsigned short WmiClassObject::UShort(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.uiVal;
}

const unsigned int WmiClassObject::UInt32(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.uintVal;
}

const unsigned long WmiClassObject::ULong(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.ulVal;
}

const unsigned long long WmiClassObject::UInt64(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.ullVal;
}

const std::wstring WmiClassObject::String(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return std::wstring(vtProp.bstrVal);
}

const std::wstring WmiClassObject::StringOrEmpty(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return std::wstring(vtProp.bstrVal != nullptr ? vtProp.bstrVal : L"");
}
