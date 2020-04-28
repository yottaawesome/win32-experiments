#include <stdexcept>
#include <Windows.h>
#include "WinHttp.hpp"

namespace WinHttp
{
	WinHttpHandle::WinHttpHandle()
		: m_handle(nullptr)
	{ }

	WinHttpHandle::WinHttpHandle(HINTERNET handle)
		: m_handle(handle)
	{ }

	void WinHttpHandle::operator=(const HINTERNET& handle)
	{
		m_handle = handle;
	}

	HINTERNET WinHttpHandle::Get()
	{
		return m_handle;
	}

	bool WinHttpHandle::operator==(const HINTERNET other)
	{
		return m_handle == other;
	}

	WinHttpHandle::~WinHttpHandle()
	{
		if (m_handle != nullptr)
			WinHttpCloseHandle(m_handle);
	}
}