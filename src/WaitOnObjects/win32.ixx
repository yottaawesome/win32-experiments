module;

#include <Windows.h>

export module win32;

export namespace Win32
{
	using
		::HANDLE,
		::DWORD,
		::WaitForSingleObject,
		::WaitForSingleObjectEx,
		::WaitForMultipleObjects,
		::WaitForMultipleObjectsEx,
		::GetLastError,
		::CloseHandle,
		::FormatMessageA,
		::FormatMessageW,
		::CreateEventA,
		::ResetEvent,
		::SetEvent
		;

	constexpr auto Infinite = INFINITE;
	constexpr auto WaitObject0 = WAIT_OBJECT_0;
	constexpr auto WaitAbandoned0 = WAIT_ABANDONED_0;
	constexpr auto WaitTimeout = WAIT_TIMEOUT;
	constexpr auto WaitFailed = WAIT_FAILED;
}
