#include "pch.h"
#include <stdexcept>
#include <iostream>
#include "include/ClientServerLib.hpp"

namespace ClientServerLib
{
	IntArray::~IntArray() {}

	IntArray::IntArray(const std::wstring& name, const UINT maxSize, const bool createNewMutex)
	:	m_mmf(name, maxSize, createNewMutex), m_currentCount(0), m_maxElements(10)
	{
		if (m_mmf.Initialised() == false)
			throw std::runtime_error("Failed array vector");
		m_array = (int*)m_mmf.GetViewPointer();
		//int* i = new(m_mmf.GetViewPointer()) int[10];
	}

	void IntArray::SetAt(const int index, const int value)
	{
		if(index >= m_maxElements)
			throw std::runtime_error("Out of range");

		m_mmf.Lock();
		m_array[index] = value;
		m_mmf.Unlock();
	}

	int IntArray::operator[](const int index)
	{
		if (index >= m_maxElements)
			throw std::runtime_error("Out of range");

		m_mmf.Lock();
		return m_array[index];
		m_mmf.Unlock();
	}

	/*void IntArray::Add(const int value)
	{
		if (m_currentCount >= m_maxElements)
			throw std::runtime_error("Out of capacity");
		m_currentCount++;
	}*/
}
