module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <process.h>

export module win32;

export namespace win32
{
	using 
		::HANDLE,
		::uintptr_t,
		::DWORD;

	using 
		::_beginthreadex, 
		::WaitForSingleObject,
		::WaitForSingleObjectEx,
		::CloseHandle,
		::GetLastError;

	constexpr auto InfiniteWait = INFINITE;
}