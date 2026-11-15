module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module Win32;

export namespace Win32
{
	using
		::HANDLE,
		::DWORD,
		::PDWORD,
		::HINSTANCE,
		::PTOKEN_PRIVILEGES,
		::LPWSTR,
		::SERVICE_TABLE_ENTRY,
		::LPSERVICE_MAIN_FUNCTIONW,
		::SC_HANDLE,
		::WCHAR,
		::SERVICE_STATUS,
		::SERVICE_STATUS_HANDLE,
		::WTSSESSION_NOTIFICATION,
		::LPVOID,
		::USHORT,
		::PROCESS_INFORMATION,
		::STARTUPINFO,
		::HKEY,
		::LONG,
		::WNDPROC,
		::WNDCLASSEXW,
		::HWND,
		::LPARAM,
		::WPARAM,
		::LRESULT,
		::LONG_PTR,
		::CREATESTRUCT,
		::MSG,
		::HBRUSH,
		::DestroyWindow,
		::GetStockObject,
		::GetMessageW,
		::TranslateMessage,
		::DispatchMessageW,
		::SetWindowLongPtrW,
		::PostQuitMessage,
		::DefWindowProcW,
		::GetWindowLongPtrW,
		::SetWindowLongPtrW,
		::RegisterClassExW,
		::CreateWindowExW,
		::GetLastError,
		::SetEvent,
		::FormatMessageW,
		::GetModuleHandleW,
		::LocalFree
		;

	namespace Brushes
	{
		enum
		{
			White = WHITE_BRUSH,
			Black = BLACK_BRUSH,
			Gray = GRAY_BRUSH,
			LightGray = LTGRAY_BRUSH,
			DarkGray = DKGRAY_BRUSH
		};
	}

	namespace WindowStyles
	{
		enum
		{
			OverlappedWindow = WS_OVERLAPPEDWINDOW,
			Visible = WS_VISIBLE
		};
	}

	namespace FormatMessageFlags
	{
		enum
		{
			AllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER,
			FromSystem = FORMAT_MESSAGE_FROM_SYSTEM,
			IgnoreInserts = FORMAT_MESSAGE_IGNORE_INSERTS
		};
	}

	namespace Gwlp
	{
		enum
		{
			UserData = GWLP_USERDATA
		};
	}

	namespace Messages
	{
		enum
		{
			NonClientCreate = WM_NCCREATE,
			NonClientDestroy = WM_NCDESTROY,
			Destroy = WM_DESTROY
		};
	}
}