#include "Header.hpp"
#include <iostream>

WbemClassObject::WbemClassObject(IWbemClassObject* clsObj)
	: m_clsObj(clsObj)
{ }

const short WbemClassObject::Short(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.iVal;
}

const int WbemClassObject::Int32(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.intVal;
}

const long WbemClassObject::Long(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.lVal;
}

const long long WbemClassObject::Int64(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.llVal;
}

const unsigned short WbemClassObject::UnsignedShort(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.uiVal;
}

const unsigned int WbemClassObject::UnsignedInt32(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.uintVal;
}

const unsigned long WbemClassObject::UnsignedLong(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.ulVal;
}

const unsigned long long WbemClassObject::UnsignedInt64(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.ullVal;
}

const std::wstring WbemClassObject::String(const std::wstring& name)
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name.c_str(), 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return std::wstring(vtProp.bstrVal);
}