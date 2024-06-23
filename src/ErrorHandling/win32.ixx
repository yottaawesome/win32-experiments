module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>

export module win32;

export namespace Win32
{
	using
		::PEXCEPTION_POINTERS,
		::PVECTORED_EXCEPTION_HANDLER,
		::_EXCEPTION_POINTERS,
		::DWORD,
		::HMODULE,
		::LPCTSTR,
		::GetModuleHandleW,
		::FormatMessageA,
		::GetLastError,
		::AddVectoredExceptionHandler,
		::LocalFree,
		::GetModuleHandleExW,
		::GetModuleFileNameW,
		::GetModuleFileNameA,
		::GetCurrentProcess,
		::K32GetModuleFileNameExA,
		::K32GetModuleFileNameExW
		;

	constexpr auto Continue = EXCEPTION_CONTINUE_SEARCH;
	constexpr auto FlagFromAddress = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS;

	namespace FormatOptions
	{
		constexpr auto AllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER;
		constexpr auto FromSystem = FORMAT_MESSAGE_FROM_SYSTEM;
		constexpr auto IgnoreInserts = FORMAT_MESSAGE_IGNORE_INSERTS;
		constexpr auto FromHModule = FORMAT_MESSAGE_FROM_HMODULE;
	}
}