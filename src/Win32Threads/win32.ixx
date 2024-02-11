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
		::DWORD,
		::ULONG_PTR,
		::SYNCHRONIZATION_BARRIER;

	using 
		::_beginthreadex, 
		::WaitForSingleObject,
		::WaitForSingleObjectEx,
		::CloseHandle,
		::GetLastError,
		::Sleep,
		::SleepEx,
		::QueueUserAPC,
		::QueueUserAPC2,
		::InitializeSynchronizationBarrier,
		::EnterSynchronizationBarrier,
		::DeleteSynchronizationBarrier,
		::CreateEventW,
		::SetEvent;

	constexpr auto InfiniteWait = INFINITE;
	constexpr auto WaitIoCompletion = WAIT_IO_COMPLETION;
}