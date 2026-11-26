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
		::NMHDR,
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
		::PBRANGE,
		::DWORD_PTR,
		::ULONG_PTR,
		::UINT_PTR,
		::RECT,
		::HFONT,
		::NONCLIENTMETRICS,
		::BOOL,
		::ShowWindow,
		::SetWindowPos,
		::GetParent,
		::SetParent,
		::EnumChildWindows,
		::SystemParametersInfoW,
		::CreateFontW,
		::CreateFontIndirectW,
		::DeleteObject,
		::GetClientRect,
		::DefSubclassProc,
		::SetWindowSubclass,
		::RemoveWindowSubclass,
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
	
	constexpr auto DefaultCharset = DEFAULT_CHARSET;
	constexpr auto SpiGetNonClientMetrics = SPI_GETNONCLIENTMETRICS;

	namespace ShowWindowFlags
	{
		enum
		{
			Show = SW_SHOW,
			Hide = SW_HIDE
		};
	}

	namespace SetWindowPosFlags
	{
		enum
		{
			NoZOrder = SWP_NOZORDER,
			NoActivate = SWP_NOACTIVATE
		};
	}

	namespace StatusBar
	{
		enum Styles
		{
			SizeGrip = SBARS_SIZEGRIP,
			ToolTips = SBARS_TOOLTIPS,
		};

		enum Messages
		{
			SetParts = SB_SETPARTS,
			GetParts = SB_GETPARTS,
			SetText = SB_SETTEXTW,
			GetText = SB_GETTEXTW,
			SetTextLength = SB_GETTEXTLENGTHW,
			GetTextLength = SB_GETTEXTLENGTHW,
			SetIcon = SB_SETICON,
			GetIcon = SB_GETICON,
			SetTipText = SB_SETTIPTEXTW,
			GetTipText = SB_GETTIPTEXTW,
		};
	}

	namespace OutPrecision
	{
		enum
		{
			OutOutlinePrecision = OUT_OUTLINE_PRECIS
		};
	}

	namespace ClipPrecision
	{
		enum
		{
			DefaultPrecision = CLIP_DEFAULT_PRECIS
		};
	}

	namespace FontQuality
	{
		enum
		{
			ClearType = CLEARTYPE_QUALITY
		};
	}

	namespace Pitch
	{
		enum
		{
			Variable = VARIABLE_PITCH
		};
	}

	namespace FontFamily
	{
		enum
		{
			Swiss = FF_SWISS
		};
	}

	namespace FontWeight
	{
		enum
		{
			Normal = FW_NORMAL
		};
	}

	namespace WindowClasses
	{
		constexpr auto ProgressBar = PROGRESS_CLASS;
		constexpr auto Button = WC_BUTTON;
		constexpr auto Static = WC_STATIC;
		constexpr auto ListView = WC_LISTVIEW;
		constexpr auto StatusBar = STATUSCLASSNAMEW;
		constexpr auto TrackBar = TRACKBAR_CLASS;
	}

	namespace Button
	{
		enum Styles
		{
			DefPushButton = BS_DEFPUSHBUTTON,
			PushButton = BS_PUSHBUTTON,
		};
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
			ExitSizeMove = WM_EXITSIZEMOVE,
			Size = WM_SIZE,
			SetFont = WM_SETFONT,

			ReflectedCommand = WM_APP + 1,
			ReflectedNotify = WM_APP + 2,
			ReflectedStaticControlColor = WM_APP + 3
		};
	}

	namespace WindowStyles
	{
		constexpr auto OverlappedWindow = WS_OVERLAPPEDWINDOW;
		constexpr auto Child = WS_CHILD;
		constexpr auto Visible = WS_VISIBLE;
		constexpr auto Disabled = WS_DISABLED;
		constexpr auto ClipChildren = WS_CLIPCHILDREN;
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
