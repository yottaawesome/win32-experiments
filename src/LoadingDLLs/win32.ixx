module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module win32;

export namespace Win32
{
	using
		::DWORD,
		::PVOID,
		::HMODULE,
		::HINSTANCE,
		::FARPROC,
		::INT_PTR,
		::HINSTANCE__,
		::LoadLibraryA,
		::LoadLibraryW,
		::LoadLibraryExA,
		::LoadLibraryExW,
		::FreeLibrary,
		::CloseHandle,
		::GetLastError,
		::GetProcAddress
		;
}
