module;

#include <windows.h>
#include <commctrl.h>

export module Win32;
import std;

export namespace Win32
{
	template<auto VValue>
	struct Constant
	{
		using TValue = decltype(
			[] 
			{
				if constexpr (std::invocable<decltype(VValue)>)
					return VValue();
				else
					return VValue;
			}());

		constexpr static auto operator()() noexcept -> TValue
		{
			if constexpr (std::invocable<decltype(VValue)>)
				return VValue();
			else
				return VValue;
		}

		operator TValue() const noexcept
		{
			return operator()();
		}
	};

	using
		::HANDLE,
		::DWORD,
		::PDWORD,
		::HINSTANCE,
		::HMODULE,
		::LPWSTR,
		::WCHAR,
		::WTSSESSION_NOTIFICATION,
		::LPVOID,
		::MSG,
		::UINT,
		::HWND,
		::WPARAM,
		::LPARAM,
		::USHORT,
		::HBRUSH,
		::STARTUPINFO,
		::LRESULT,
		::WNDCLASSEXW,
		::CREATESTRUCTW,
		::LONG_PTR,
		::HICON,
		::HDC,
		::LPSTR,
		::HMENU,
		::PAINTSTRUCT,
		::DWORD_PTR,
		::ULONG_PTR,
		::UINT_PTR,
		::DefSubclassProc,
		::SetWindowSubclass,
		::InitCommonControls,
		::OutputDebugStringA,
		::OutputDebugStringW,
		::SendMessageW,
		::BeginPaint,
		::EndPaint,
		::DestroyWindow,
		::CreateWindowExW,
		::FormatMessageA,
		::GetLastError,
		::LocalFree,
		::RegisterClassExW,
		::GetWindowLongPtrW,
		::SetWindowLongPtrW,
		::DefWindowProcW,
		::PostQuitMessage,
		::LoadCursorW,
		::GetMessageW,
		::TranslateMessage,
		::GetModuleHandleW,
		::DispatchMessageW
		;
	
	namespace WindowClasses
	{
		constexpr auto ProgressBar = PROGRESS_CLASS;
	}

	namespace ProgressBar
	{
		enum Styles
		{
			Smooth = PBS_SMOOTH,
			Vertical = PBS_VERTICAL,
			Marquee = PBS_MARQUEE,
		};

		namespace Messages
		{
			enum
			{
				SetRange = PBM_SETRANGE32,
				SetPos = PBM_SETPOS,
				SetStep = PBM_SETSTEP,
				StepIt = PBM_STEPIT,
				SetMarquee = PBM_SETMARQUEE,
				GetPos = PBM_GETPOS,
				GetRange = PBM_GETRANGE,
				GetStep = PBM_GETSTEP,
				GetState = PBM_GETSTATE,
				SetState = PBM_SETSTATE,
				DeltaPos = PBM_DELTAPOS,
			};
		}

		namespace States
		{
			enum
			{
				Normal = PBST_NORMAL,
				Error = PBST_ERROR,
				Paused = PBST_PAUSED
			};
		}
	}

	namespace ErrorCodes
	{
		enum
		{
			ClassAlreadyExists = ERROR_CLASS_ALREADY_EXISTS
		};
	}

	namespace WindowMessages
	{
		enum
		{
			Create = WM_CREATE,
			Destroy = WM_DESTROY,
			Close = WM_CLOSE,
			Command = WM_COMMAND,
			Paint = WM_PAINT,
			Notify = WM_NOTIFY,
			StaticControlColor = WM_CTLCOLORSTATIC,
			NonClientCreate = WM_NCCREATE,
			NonClientDestroy = WM_NCDESTROY,

			ReflectedCommand = WM_APP + 1,
			ReflectedNotify = WM_APP + 2,
			ReflectedStaticControlColor = WM_APP + 3
		};
	}

	namespace WindowStyles
	{
		constexpr auto WsOverlappedWindow = WS_OVERLAPPEDWINDOW;
		constexpr auto WsChild = WS_CHILD;
		constexpr auto WsVisible = WS_VISIBLE;
		constexpr auto WsDisabled = WS_DISABLED;
	}

	namespace CreateWindowConstants
	{
		constexpr auto UseDefault = CW_USEDEFAULT;
	}

	constexpr auto IdcArrow = Constant<IDC_ARROW>{};
	constexpr auto ColorWindow = COLOR_WINDOW;

	namespace Gwl
	{
		enum
		{
			Style = GWL_STYLE
		};
	}

	namespace  Gwlp
	{
		enum
		{
			UserData = GWLP_USERDATA
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
}
