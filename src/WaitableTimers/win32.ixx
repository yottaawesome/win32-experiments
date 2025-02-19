module;

#include <Windows.h>

export module win32;

export namespace Win32
{
	using
		::DWORD,
		::LPSTR,
		::HANDLE,
		::LARGE_INTEGER,
		::BOOL,
		::CloseHandle,
		::CreateWaitableTimerW,
		::CreateWaitableTimerExW,
		::SetWaitableTimer,
		::SetWaitableTimerEx,
		::CancelWaitableTimer,
		::WaitForSingleObject,
		::WaitForSingleObjectEx,
		::GetLastError,
		::FormatMessageA,
		::FormatMessageW,
		::LocalFree,
		::QueueUserAPC,
		::QueueUserAPC2
		;

	constexpr auto Infinite = INFINITE;

	namespace WaitConstants
	{
		enum
		{
			Timeout = WAIT_TIMEOUT,
			Object0 = WAIT_OBJECT_0,
			Failed = WAIT_FAILED,
			Abandoned = WAIT_ABANDONED,
			IoCompletion = WAIT_IO_COMPLETION
		};
	}

	namespace FormatFlags
	{
		enum
		{
			AllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER,
			FromSystem = FORMAT_MESSAGE_FROM_SYSTEM,
			IgnoreInserts =FORMAT_MESSAGE_IGNORE_INSERTS
		};
	}
}
