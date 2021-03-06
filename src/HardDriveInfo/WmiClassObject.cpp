#include "Header.hpp"
#include <iostream>
#include <sstream>

//https://docs.microsoft.com/en-us/windows/win32/api/propidlbase/ns-propidlbase-propvariant
WmiClassObject::WmiClassObject(IWbemClassObject* clsObj)
	: m_clsObj(clsObj)
{ }

WmiClassObject::~WmiClassObject()
{
	if (m_clsObj)
		m_clsObj->Release();
}

const short WmiClassObject::Short(const wchar_t* name) const
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name, 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");
	if (vtProp.vt != VT_I2)
		throw new std::runtime_error("WMI value is not typed as a VT_I2.");

	return vtProp.iVal;
}

const bool WmiClassObject::Bool(const wchar_t* name) const
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name, 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");
	if (vtProp.vt != VT_BOOL)
		throw new std::runtime_error("WMI value is not typed as a VT_BOOL.");

	return vtProp.iVal;
}

const int WmiClassObject::Int32(const wchar_t* name) const
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name, 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");
	if (vtProp.vt != VT_I4)
		throw new std::runtime_error("WMI value is not typed as a VT_I4.");

	return vtProp.intVal;
}

const BYTE WmiClassObject::Byte(const wchar_t* name) const
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name, 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");
	if (vtProp.vt != VT_UI1)
		throw new std::runtime_error("WMI value is not typed as a VT_I1.");

	return vtProp.bVal;
}

const long WmiClassObject::Long(const wchar_t* name) const
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name, 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");
	if (vtProp.vt != VT_I4)
		throw new std::runtime_error("WMI value is not typed as a VT_I4.");

	return vtProp.lVal;
}

const long long WmiClassObject::Int64(const wchar_t* name) const
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name, 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");
	if (vtProp.vt != VT_I8)
		throw new std::runtime_error("WMI value is not typed as a VT_I8.");
	return vtProp.llVal;
}

const unsigned short WmiClassObject::UShort(const wchar_t* name) const
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name, 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");
	if (vtProp.vt != VT_UI2)
		throw new std::runtime_error("WMI value is not typed as a VT_UI2.");
	return vtProp.uiVal;
}

const unsigned int WmiClassObject::UInt32(const wchar_t* name) const
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name, 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");
	if (vtProp.vt != VT_UI4)
		throw new std::runtime_error("WMI value is not typed as a VT_UI4.");
	return vtProp.uintVal;
}

const unsigned long WmiClassObject::ULong(const wchar_t* name) const
{
	_variant_t vtProp;

	HRESULT hr = m_clsObj->Get(name, 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");
	if (vtProp.vt != VT_UI4)
		throw new std::runtime_error("WMI value is not typed as a VT_UI4.");
	return vtProp.ulVal;
}

const unsigned long long WmiClassObject::UInt64(const wchar_t* name) const
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name, 0, &vtProp, 0, 0);
	if (vtProp.vt != VT_UI8)
		throw new std::runtime_error("WMI value is not typed as a VT_UI8.");
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");

	return vtProp.ullVal;
}

const std::vector<std::wstring> WmiClassObject::StringVector(const wchar_t* name) const
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name, 0, &vtProp, 0, 0);
	Util::CheckHr(hr, "Failed ClassObject::Get()");

	std::vector<std::wstring> results;
	if (vtProp.vt != (VT_ARRAY | VT_BSTR))
		throw new std::runtime_error("WMI value is not typed as a BSTR.");
	
	SAFEARRAY* addrArray = vtProp.parray;
	// get array bounds
	long lowerBound = 0;
	long upperBound = 0; 
	SafeArrayGetLBound(addrArray, 1, &lowerBound);
	SafeArrayGetUBound(addrArray, 1, &upperBound);

	long elementCount = upperBound - lowerBound + 1;
	for (long i = 0; i < elementCount; i++)
	{
		BSTR addr;
		SafeArrayGetElement(vtProp.parray, &i, &addr);
		results.push_back(std::wstring(addr, SysStringLen(addr)));
	}

	return results;
}

const std::wstring WmiClassObject::String(const wchar_t* name) const
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name, 0, &vtProp, 0, 0);
	Util::CheckHr(hr, "Failed ClassObject::Get()");
	if (vtProp.vt != VT_BSTR && vtProp.vt != VT_NULL && vtProp.vt != VT_EMPTY)
		throw new std::runtime_error("WMI value is not typed as a BSTR.");
	if (vtProp.bstrVal == nullptr)
		return L"";

	return std::wstring(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
}

// TODO ugly, find a better way
const uint64_t WmiClassObject::StringAsUInt64(const wchar_t* name) const
{
	_variant_t vtProp;
	HRESULT hr = m_clsObj->Get(name, 0, &vtProp, 0, 0);
	Util::CheckHr(hr, "Failed ClassObject::Get()");
	if (vtProp.vt != VT_BSTR)
		throw new std::runtime_error("WMI value is not typed as a BSTR.");
	if (vtProp.bstrVal == nullptr)
		return 0;

	std::wstring str(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));

	uint64_t returnVal;
	std::wistringstream ss(str);
	ss >> returnVal;
	if (ss.fail())
	{
		std::stringstream ss;
		ss
			<< "Failed to convert value to UINT64. Value: "
			<< Util::ConvertWStringToString(str);
		throw std::runtime_error(ss.str());
	}

	return returnVal;
}

const void* WmiClassObject::ObjectRef(const wchar_t* name) const
{
	VARIANT vtProp;
	VariantInit(&vtProp);
	HRESULT hr = m_clsObj->Get(name, 0, &vtProp, 0, 0);
	if (FAILED(hr))
		throw new std::runtime_error("Failed ClassObject::Get()");
	if (vtProp.vt != VT_BYREF)
		throw new std::runtime_error("WMI value is not typed as a BSTR.");
	// We don't clean up the variant here, because this is a byref object.
	// As such, clearing the variant would release the object prematurely.
	// It is up to the client code to release the object, as appropriate.
	return vtProp.byref;
}
