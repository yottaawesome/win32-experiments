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
		::HMODULE,
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
		::ATOM,
		::GetWindowRect,
		::ShowWindow,
		::MultiByteToWideChar,
		::WideCharToMultiByte,
		::SetWindowTextW,
		::DeleteObject,
		::CreateFontIndirectW,
		::SystemParametersInfoW,
		::EnumChildWindows,
		::SendMessageW,
		::PostMessageW,
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
		::SetLastError,
		::SetEvent,
		::FormatMessageW,
		::GetModuleHandleW,
		::GetModuleHandleExW,
		::GetModuleFileNameW,
		::GetTempPathW,
		::RtlCaptureStackBackTrace,
		::LocalFree
		;

	constexpr auto SpiGetNonClientMetrics = SPI_GETNONCLIENTMETRICS;

	namespace GetModuleHandleFlags
	{
		enum
		{
			FromAddress = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
			UnchangedRefCount = GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
		};
	}

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

	namespace ShowWindowFlags
	{
		enum
		{
			Show = SW_SHOW,
			Hide = SW_HIDE,
			ShowDefault = SW_SHOWNORMAL,
			ShowMinimized = SW_SHOWMINIMIZED,
			ShowMaximized = SW_SHOWMAXIMIZED
		};
	}

	namespace ExtendedWindowStyles
	{
		enum
		{
			TopMost = WS_EX_TOPMOST,
			NoActivate = WS_EX_NOACTIVATE,
			ToolWindow = WS_EX_TOOLWINDOW,
			AppWindow = WS_EX_APPWINDOW,
			Transparent = WS_EX_TRANSPARENT,
			Composited = WS_EX_COMPOSITED,
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
			ThickFrame = WS_THICKFRAME,
			MaximizeBox = WS_MAXIMIZEBOX,
			ClipSiblings = WS_CLIPSIBLINGS,
			ClipChildren = WS_CLIPCHILDREN
		};
	}

	namespace WindowClasses
	{
		constexpr auto Button = WC_BUTTON;
		constexpr auto Edit = WC_EDIT;
		constexpr auto ListBox = WC_LISTBOX;
		constexpr auto ScrollBar = WC_SCROLLBAR;
		constexpr auto Tab = WC_TABCONTROL;
		constexpr auto Static = WC_STATIC;
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

	[[nodiscard]]
	constexpr auto HiWord(auto value) noexcept -> auto
	{
		return HIWORD(value);
	}
	[[nodiscard]]
	constexpr auto LoWord(auto value) noexcept -> auto
	{
		return LOWORD(value);
	}

	namespace Messages
	{
		enum
		{
			NonClientCreate = WM_NCCREATE,
			NonClientDestroy = WM_NCDESTROY,
			Destroy = WM_DESTROY,
			Close = WM_CLOSE,
			CtlColorStatic = WM_CTLCOLORSTATIC,
			SetFont = WM_SETFONT,
			Command = WM_COMMAND,
			ReflectedCommand = WM_APP + 1
		};
	}

	namespace BN
	{
		enum
		{
			Clicked = BN_CLICKED
		};
	}
}