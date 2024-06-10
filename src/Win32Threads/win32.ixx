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
		::SYNCHRONIZATION_BARRIER,
		::MSG,
		::BOOL,
		::WNDCLASSEXW,
		::LPARAM,
		::WPARAM,
		::LRESULT,
		::UINT32,
		::HWND,
		::ATOM
		;

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
		::PeekMessageW,
		::PostThreadMessageW,
		::GetMessageW,
		::GetCurrentThreadId,
		::SetEvent,
		::RegisterClassExW,
		::GetModuleHandleW,
		::DefWindowProcW,
		::PostQuitMessage,
		::CreateWindowExW,
		::SendMessageW,
		::TranslateMessage,
		::DispatchMessageW,
		::ShowWindow,
		::PostMessageW
		;


	namespace WaitResults
	{
		constexpr auto Index = WAIT_OBJECT_0;
		constexpr auto Timeout = WAIT_TIMEOUT;
	}

	constexpr auto NoRemove = PM_NOREMOVE;
	constexpr auto InfiniteWait = INFINITE;
	constexpr auto WaitIoCompletion = WAIT_IO_COMPLETION;

	constexpr auto Hide = SW_HIDE;

	namespace Msg
	{
		constexpr auto MessageBase = WM_APP;
		constexpr auto Close = WM_CLOSE;
		constexpr auto QuitMsg = WM_QUIT;
	}
}