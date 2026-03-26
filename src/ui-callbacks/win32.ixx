module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module uicallbacks:win32;

export namespace Win32
{
	using
		::HANDLE,
		::HINSTANCE,
		::LPWSTR,
		::UINT,
		::MSG,
		::LPARAM,
		::WPARAM,
		::WNDCLASSEXW,
		::HWND,
		::LRESULT,
		::DWORD,
		::LPSTR,
		::LONG_PTR,
		::CREATESTRUCT,
		::WORD,
		::MessageBoxW,
		::EnableWindow,
		::SetWindowTextW,
		::DispatchMessageW,
		::UpdateWindow,
		::ShowWindow,
		::GetModuleHandleW,
		::GetWindowLongPtrW,
		::DestroyWindow,
		::PostQuitMessage,
		::TranslateMessage,
		::SetWindowLongPtrW,
		::LocalFree,
		::GetLastError,
		::FormatMessageA,
		::FormatMessageW,
		::DefWindowProcW,
		::GetMessageW,
		::PeekMessageW,
		::WaitMessage,
		::CreateWindowExW,
		::RegisterClassExW
		;

	namespace MessageBoxOptions
	{
		enum
		{
			Ok = MB_OK,
			OkCancel = MB_OKCANCEL,
			AbortRetryIgnore = MB_ABORTRETRYIGNORE,
			YesNoCancel = MB_YESNOCANCEL,
			YesNo = MB_YESNO,
			RetryCancel = MB_RETRYCANCEL,
			Critical = MB_ICONERROR,
			Question = MB_ICONQUESTION,
			Exclamation = MB_ICONEXCLAMATION,
			Information = MB_ICONINFORMATION
		};
	}

	namespace ErrorCodes
	{
		enum
		{
			Success = ERROR_SUCCESS,
			FileNotFound = ERROR_FILE_NOT_FOUND,
			AccessDenied = ERROR_ACCESS_DENIED,
			InvalidHandle = ERROR_INVALID_HANDLE,
			NotEnoughMemory = ERROR_NOT_ENOUGH_MEMORY,
			ClassDoesNotExist = ERROR_CLASS_DOES_NOT_EXIST,
			InvalidParameter = ERROR_INVALID_PARAMETER,
			ClassAlreadyExists = ERROR_CLASS_ALREADY_EXISTS,
		};
	}

	namespace ButtonMessages
	{
		enum
		{
			Clicked = BN_CLICKED
		};
	}

	namespace FormatMessageFlags
	{
		enum
		{
			AllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER,
			IgnoreInserts = FORMAT_MESSAGE_IGNORE_INSERTS,
			FromString = FORMAT_MESSAGE_FROM_STRING,
			FromHModule = FORMAT_MESSAGE_FROM_HMODULE,
			FromSystem = FORMAT_MESSAGE_FROM_SYSTEM,
			ArgumentArray = FORMAT_MESSAGE_ARGUMENT_ARRAY
		};
	}

	namespace WindowMessages
	{
		enum
		{
			Close = WM_CLOSE,
			Destroy = WM_DESTROY,
			NonClientDestroy = WM_NCDESTROY,
			Size = WM_SIZE,
			NonClientCreate = WM_NCCREATE,
			Quit = WM_QUIT,
			Command = WM_COMMAND
		};
	}

	constexpr auto GetLowWord(LPARAM lParam) -> int
	{
		return static_cast<int>(lParam & 0xFFFF);
	}

	constexpr auto GetHighWord(LPARAM lParam) -> int
	{
		return static_cast<int>((lParam >> 16) & 0xFFFF);
	}

	namespace Gwlp
	{
		enum
		{
			UserData = GWLP_USERDATA,
			WndProc = GWLP_WNDPROC,
			HInstance = GWLP_HINSTANCE,
			HwndParent = GWLP_HWNDPARENT,
			ID = GWLP_ID
		};
	}

	namespace WindowStyles
	{
		enum
		{
			Overlapped = WS_OVERLAPPED,
			Popup = WS_POPUP,
			Child = WS_CHILD,
			Minimize = WS_MINIMIZE,
			Visible = WS_VISIBLE,
			Disabled = WS_DISABLED,
			ClipSiblings = WS_CLIPSIBLINGS,
			ClipChildren = WS_CLIPCHILDREN,
			Maximize = WS_MAXIMIZE,
			Caption = WS_CAPTION,
			Border = WS_BORDER,
			DlgFrame = WS_DLGFRAME,
			VScroll = WS_VSCROLL,
			HScroll = WS_HSCROLL,
			SysMenu = WS_SYSMENU,
			ThickFrame = WS_THICKFRAME,
			Group = WS_GROUP,
			TabStop = WS_TABSTOP,
			MinimizeBox = WS_MINIMIZEBOX,
			MaximizeBox = WS_MAXIMIZEBOX,
			OverlappedWindow = WS_OVERLAPPEDWINDOW,

			PopupWindow = WS_POPUPWINDOW,
			PushButton = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON
		};
	}

	namespace CwConstants
	{	
		enum
		{
			UseDefault = CW_USEDEFAULT
		};
	}

	namespace ShowWindowOptions
	{
		enum
		{
			Hide = SW_HIDE,
			ShowNormal = SW_SHOWNORMAL,
			ShowMinimized = SW_SHOWMINIMIZED,
			ShowMaximized = SW_SHOWMAXIMIZED,
			ShowNoActivate = SW_SHOWNOACTIVATE,
			Show = SW_SHOW,
			Minimize = SW_MINIMIZE,
			ShowMinNoActive = SW_SHOWMINNOACTIVE,
			ShowNA = SW_SHOWNA,
			Restore = SW_RESTORE,
			Default = SW_SHOWDEFAULT,
			ForceMinimize = SW_FORCEMINIMIZE
		};
	}

	namespace PeekMessageOptions
	{
		enum
		{
			Remove = PM_REMOVE,
			NoRemove = PM_NOREMOVE,
			NoYield = PM_NOYIELD
		};
	}
}
