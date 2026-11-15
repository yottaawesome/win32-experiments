module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commctrl.h>

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
		::RECT,
		::ULONG_PTR,
		::HMENU,
		::DWORD_PTR,
		::UINT_PTR,
		::HFONT,
		::NONCLIENTMETRICS,
		::BOOL,
		::MultiByteToWideChar,
		::WideCharToMultiByte,
		::SetWindowTextW,
		::DeleteObject,
		::CreateFontIndirectW,
		::SystemParametersInfoW,
		::EnumChildWindows,
		::SendMessageW,
		::RemoveWindowSubclass,
		::DefSubclassProc,
		::SetWindowSubclass,
		::GetClientRect,
		::SetWindowPos,
		::UnregisterClassW,
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

	constexpr auto SpiGetNonClientMetrics = SPI_GETNONCLIENTMETRICS;

	namespace CodePage
	{
		constexpr auto Utf8 = CP_UTF8;
	}

	namespace SetWindowPosFlags
	{
		constexpr auto NoZOrder = SWP_NOZORDER;
		constexpr auto NoActivate = SWP_NOACTIVATE;
		constexpr auto NoMove = SWP_NOMOVE;
		constexpr auto NoSize = SWP_NOSIZE;
	}

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
			Visible = WS_VISIBLE,
			Child = WS_CHILD,
			Border = WS_BORDER,
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

	namespace SS
	{
		enum
		{
			NoChange = SS_NOPREFIX,
			NoPrefix = SS_NOPREFIX,
			Center = SS_CENTER,
			Left = SS_LEFT,
			Right = SS_RIGHT,
			Simple = SS_SIMPLE,
			LeftNoWordWrap = SS_LEFTNOWORDWRAP,
			OwnerDraw = SS_OWNERDRAW,
			TypeMask = SS_TYPEMASK,
			CenterImage = SS_CENTERIMAGE,
			EditControl = SS_EDITCONTROL,
			EndEllipsis = SS_ENDELLIPSIS,
			EllipsisMask = SS_ELLIPSISMASK,
		};
	}

	namespace Messages
	{
		enum
		{
			NonClientCreate = WM_NCCREATE,
			NonClientDestroy = WM_NCDESTROY,
			Destroy = WM_DESTROY,
			CtlColorStatic = WM_CTLCOLORSTATIC,
			SetFont = WM_SETFONT,
		};
	}
}