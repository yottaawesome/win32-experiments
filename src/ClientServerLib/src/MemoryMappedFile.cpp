#include "pch.h"
#include <stdexcept>
#include <iostream>
#include "include/ClientServerLib.hpp"

namespace ClientServerLib
{
	MemoryMappedFile::MemoryMappedFile(const std::wstring& name, const UINT maxSize, const bool createNewMutex)
	:	m_mmfName(name),
		m_mutexName(name + L"Mutex"),
		m_maxSize(maxSize),
		m_MapFile(nullptr),
		m_Mutex(nullptr),
		m_createNewMutex(createNewMutex),
		m_initialised(false),
		m_locked(false)
	{
		if (createNewMutex)
		{
			SECURITY_ATTRIBUTES lp{ 0 };
			lp.nLength = sizeof(lp);
			lp.bInheritHandle = true;
			m_Mutex = CreateMutex(
				&lp,
				false,
				m_mutexName.c_str()
			);
		}
		else
		{
			m_Mutex = OpenMutex(SYNCHRONIZE, false, m_mutexName.c_str());
		}

		if (m_Mutex == nullptr)
		{
			Cleanup();
			std::wcout << std::to_wstring(GetLastError()) << std::endl;
			throw std::runtime_error("Failed to open memory mapped file");
		}
		
		Lock();
		if (createNewMutex)
		{
			SECURITY_ATTRIBUTES lp{ 0 };
			lp.nLength = sizeof(lp);
			lp.bInheritHandle = true;
			m_MapFile = CreateFileMapping(
				INVALID_HANDLE_VALUE,    // use paging file
				&lp,                    // default security
				PAGE_READWRITE,          // read/write access
				0,                       // maximum object size (high-order DWORD)
				m_maxSize,                // maximum object size (low-order DWORD)
				m_mmfName.c_str());                 // m_name of mapping object
		}
		else
		{
			m_MapFile = OpenFileMapping(
				FILE_MAP_ALL_ACCESS,   // read/write access
				true,                 // do not inherit the name
				m_mmfName.c_str()					// name of mapping object
			);
		}
		
		if (m_MapFile == nullptr)
		{
			std::wcout << std::to_wstring(GetLastError()) << std::endl;
			Cleanup();
			throw std::runtime_error("Failed to open memory mapped file");
		}

		m_View = (void*)MapViewOfFile(
			m_MapFile,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,
			0,
			maxSize
		);
		if (m_View == nullptr)
		{
			Cleanup();
			throw std::runtime_error("MapViewOfFile() failed");
		}

		// Don't zero the memory if we're opening an existing handle, as it may have data.
		if (createNewMutex)
		{
			ZeroMemory(m_View, m_maxSize);
		}

		m_initialised = true;
		Unlock();
	}

	void* MemoryMappedFile::GetViewPointer()
	{
		return m_View;
	}

	bool MemoryMappedFile::Initialised()
	{
		return m_initialised;
	}

	void MemoryMappedFile::Cleanup()
	{
		if (m_View)
		{
			UnmapViewOfFile(m_View);
			m_View = nullptr;
		}
		if (m_MapFile)
		{
			CloseHandle(m_MapFile);
			m_MapFile = nullptr;
		}
		if (m_Mutex)
		{
			CloseHandle(m_Mutex);
			m_Mutex = nullptr;
		}
	}

	/*MemoryMappedFile::MemoryMappedFile(const MemoryMappedFile& other)
	:	m_maxSize(other.m_maxSize),
	{
		if (other.m_initialised)
		{
			m_MapFile = DuplicateHandle(other.m_MapFile);

		}
	}*/

	void MemoryMappedFile::operator=(const MemoryMappedFile& other)
	{
		if (other.m_initialised)
			;
	}

	MemoryMappedFile::~MemoryMappedFile()
	{
		Cleanup();
	}

	void MemoryMappedFile::Lock()
	{
		WaitForSingleObject(m_Mutex, INFINITE);
		m_locked = true;
	}

	void MemoryMappedFile::Unlock()
	{
		m_locked = false;
		if (!ReleaseMutex(m_Mutex))
			std::wcout << L"Mutex release failed" << std::endl;
	}

	/*int MemoryMappedFile::operator[](const int index)
	{
		return m_vectorView[index];
	}

	void MemoryMappedFile::SetAt(const int index, const int value)
	{
		m_vectorView[index] = value;
	}*/

	/*void MemoryMappedFile::Write(const std::wstring msg)
	{
		WaitForSingleObject(m_Mutex, INFINITE);
		ZeroMemory((void*)m_View, m_maxSize);
		CopyMemory((PVOID)m_View, msg.c_str(), (msg.size() * sizeof(wchar_t)));
		ReleaseMutex(m_Mutex);
	}

	std::wstring MemoryMappedFile::Read()
	{
		WaitForSingleObject(m_Mutex, INFINITE);
		std::wstring result(m_View);
		ReleaseMutex(m_Mutex);
		return result;
	}*/
}