#pragma once
#include <string>
#include <Windows.h>
#include <vector>
#include <utility>

namespace ClientServerLib
{
	class AnonymousPipe
	{
		public:
			AnonymousPipe(const bool inheritable);
			virtual ~AnonymousPipe();

	};


	class MemoryMappedFile
	{
		public:
			//MemoryMappedFile(const MemoryMappedFile& other);
			MemoryMappedFile(const std::wstring& name, const UINT maxSize, const bool createNewMutex);
			virtual ~MemoryMappedFile();
			virtual void* GetViewPointer();
			virtual void operator=(const MemoryMappedFile& other);
			virtual bool Initialised();
			virtual void Lock();
			virtual void Unlock();

		protected:
			virtual void Cleanup();
			bool m_initialised;
			bool m_createNewMutex;
			const std::wstring m_mmfName;
			const std::wstring m_mutexName;
			const UINT m_maxSize;
			HANDLE m_MapFile;
			HANDLE m_Mutex;
			void* m_View;
			bool m_locked;
	};

	class IntArray
	{
		public:
			virtual ~IntArray();
			IntArray(const std::wstring& name, const UINT maxSize, const bool createNewMutex);
			virtual void SetAt(const int index, const int value);
			//virtual void Add(const int value);
			virtual int operator[](const int index);

		protected:
			MemoryMappedFile m_mmf;
			int* m_array;
			int m_currentCount;
			int m_maxElements;
	};

	class Message
	{
		public:
			Message(const std::wstring msg);
			std::wstring& GetMsg();
		protected:
			std::wstring m_msg;
	};

	template<typename T>
	class TypedArray
	{
		public:
			virtual ~TypedArray() {}

			TypedArray(const std::wstring& name, const bool createNewMutex)
			:	m_currentCount(nullptr),
				m_maxElements(10), 
				m_mmf(name, 10*sizeof(T), createNewMutex)
			{
				if (m_mmf.Initialised() == false)
					throw std::runtime_error("Failed array vector");
				m_mmf.Lock();
				int* i = (int*)m_mmf.GetViewPointer();
				m_currentCount = i;
				i++;
				m_array = (T*)((int*)m_mmf.GetViewPointer()+1);
				m_mmf.Unlock();
				//int* i = new(m_mmf.GetViewPointer()) int[10];
			}

			virtual void Lock()
			{
				m_mmf.Lock();
			}

			virtual void Unlock()
			{
				m_mmf.Unlock();
			}

			virtual int GetCurrentCount()
			{
				return *m_currentCount;
			}

			virtual int GetMaxCount()
			{
				return m_maxElements;
			}

			virtual T* operator[](const int index)
			{
				try
				{
					m_mmf.Lock();
					if (index >= m_maxElements || index >= *m_currentCount)
					{
						m_mmf.Unlock();
						throw std::runtime_error("Out of range");
					}

					T* ptr = &m_array[index];
					m_mmf.Unlock();
					return ptr;
				}
				catch (const std::exception& ex)
				{
					std::wcout << "Failed" << ex.what() << std::endl;
					throw;
				}
			}

			template<typename...Args>
			void Add(Args&&...args)
			{
				if (*m_currentCount >= m_maxElements)
					throw std::runtime_error("Out of range");

				m_mmf.Lock();
				new(&m_array[*m_currentCount]) T(std::forward<Args>(args)...);
				(*m_currentCount)++;
				std::wcout << "Added new element. Current count: " << *m_currentCount << std::endl;
				m_mmf.Unlock();
			}

			virtual void Clear()
			{
				if (*m_currentCount > 0)
				{
					m_mmf.Lock();
					SecureZeroMemory(m_array, m_maxElements*sizeof(T));
					*m_currentCount = 0;
					m_mmf.Unlock();
				}
			}

		protected:
			MemoryMappedFile m_mmf;
			T* m_array;
			int* m_currentCount;
			int m_maxElements;
	};

	template<typename T>
	class TypedMemoryMappedFile : public MemoryMappedFile
	{
	public:
		TypedMemoryMappedFile(const std::wstring& name, const UINT maxSize, const bool createNewMutex)
		:	MemoryMappedFile(name, maxSize, createNewMutex)
		{ }
		virtual ~TypedMemoryMappedFile() {  }
		virtual T* operator->() { return (T*)m_View; }
	};
}