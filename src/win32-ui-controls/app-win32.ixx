module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Windowsx.h>
#include <CommCtrl.h>
#include <wingdi.h>

export module app:win32;
import std;

namespace Win32
{
	template<auto VValue>
	struct Win32Constant
	{
		operator decltype(VValue)(this auto&&) noexcept
			requires (not std::invocable<decltype(VValue)>)
		{
			return VValue;
		}

		operator decltype(VValue)(this auto&&) noexcept
			requires std::invocable<decltype(VValue)>
		{
			return VValue();
		}
	};
}

export namespace Win32
{
	using
		::BOOL,
		::HMENU,
		::HANDLE,
		::UINT,
		::ATOM,
		::HINSTANCE,
		::HMODULE,
		::HWND,
		::PCWSTR,
		::PWSTR,
		::WNDCLASSW,
		::WNDCLASSEXW,
		::DWORD,
		::HLOCAL,
		::LRESULT,
		::LPARAM,
		::WPARAM,
		::LONG_PTR,
		::CREATESTRUCT,
		::HGDIOBJ,
		::HBRUSH,
		::MSG,
		::UINT32,
		::UINT_PTR,
		::DWORD_PTR,
		::INITCOMMONCONTROLSEX,
		::NONCLIENTMETRICS,
		::HFONT,
		::WORD,
		::LPWSTR,
		::PAINTSTRUCT,
		::HDC,
		::RECT,
		::POINT,
		::TRACKMOUSEEVENT,
		::COLORREF,
		::HPEN,
		::CreatePen,
		::DrawTextW,
		::InvalidateRect,
		::TrackMouseEvent,
		::PtInRect,
		::ScreenToClient,
		::GetMessagePos,
		::TextOutW,
		::SelectObject,
		::GetClientRect,
		::RoundRect,
		::BeginPaint,
		::FillRect,
		::EndPaint,
		::CreateFontW,
		::SetFocus,
		::MessageBoxA,
		::MessageBoxW,
		::DefSubclassProc,
		::SetWindowSubclass,
		::DestroyWindow,
		::PostQuitMessage,
		::PeekMessageW,
		::TranslateMessage,
		::DispatchMessageW,
		::GetMessageW,
		::TranslateMessage,
		::DispatchMessageW,
		::GetWindowLongPtrW,
		::SetWindowLongPtrW,
		::CloseHandle,
		::WaitForSingleObject,
		::WaitForSingleObjectEx,
		::WaitForMultipleObjects,
		::WaitForMultipleObjectsEx,
		::RegisterClassW,
		::RegisterClassExW,
		::SetActiveWindow,
		::ShowWindow,
		::ShowWindowAsync,
		::GetModuleHandleW,
		::GetStockObject,
		::LoadIconW,
		::LoadCursorW,
		::GetLastError,
		::FormatMessageA,
		::FormatMessageW,
		::__fastfail,
		::ExitProcess,
		::FatalAppExitW,
		::FatalExit,
		::LocalFree,
		::DefWindowProcW,
		::CreateWindowExW,
		::ShowWindow,
		::InitCommonControls,
		::AllocConsole,
		::WriteConsoleA,
		::WriteConsoleW,
		::FreeConsole,
		::InitCommonControlsEx,
		::SetWindowRgn,
		::CreateRectRgn,
		::CreateRoundRectRgn,
		::WideCharToMultiByte,
		::MultiByteToWideChar,
		::DeleteObject,
		::CreatePrivateNamespaceW,
		::CreatePrivateObjectSecurity,
		::CreateFontIndirectW,
		::SystemParametersInfoW,
		::SendMessageW,
		::SendMessageA,
		::PostMessageA,
		::PostMessageW,
		::SetWindowTextW,
		::SetWindowTextA,
		::GetWindowTextW,
		::GetWindowTextLengthW,
		::GetWindowTextA,
		::GetWindowTextLengthW,
		::GetClassInfoExA,
		::GetClassInfoExW,
		::UnregisterClassW,
		::GetStockObject,
		::SetBkMode,
		::SetTextColor,
		::CreateSolidBrush
		;

	constexpr auto PsInsideFrame = PS_INSIDEFRAME;

	constexpr auto (RGB)(BYTE r, BYTE g, BYTE b) noexcept -> COLORREF
	{
		return RGB(r, g, b);
	}
	constexpr auto (RGB)(std::byte r, std::byte g, std::byte b) noexcept -> COLORREF
	{
		return (RGB)(static_cast<BYTE>(r), static_cast<BYTE>(g), static_cast<BYTE>(b));
	}

	namespace BackgroundMode
	{
		enum
		{
			Opaque = OPAQUE,
			Transparent = TRANSPARENT
		};
	}

	namespace Pens
	{
		enum
		{
			White = WHITE_PEN,
			Black = BLACK_PEN
		};
	}

	namespace Brushes
	{
		enum
		{
			Black = BLACK_BRUSH,
			White = WHITE_BRUSH
		};
	}

	auto (SelectBrush)(HDC hdc, HGDIOBJ gdi)
	{
		return SelectBrush(hdc, gdi);
	}

	constexpr auto GetXParam(auto x) { return GET_X_LPARAM(x); }
	constexpr auto GetYParam(auto x) { return GET_Y_LPARAM(x); }

	constexpr auto Gwlp_UserData = GWLP_USERDATA;
	constexpr auto CwUseDefault = CW_USEDEFAULT;
	constexpr auto SpiGetNonClientMetrics = SPI_GETNONCLIENTMETRICS;
	constexpr auto DefaultCharset = DEFAULT_CHARSET;

	namespace DrawTextOptions
	{
		enum
		{
			Center = DT_CENTER,
			VerticalCenter = DT_VCENTER,
			SingleLine = DT_SINGLELINE
		};
	}

	namespace TrackMouseEvents
	{
		enum
		{
			Hover = TME_HOVER,
			Leave = TME_LEAVE
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

	namespace InitCommonControlsFlag
	{
		enum
		{
			Animate = ICC_ANIMATE_CLASS,
			NativeClass = ICC_NATIVEFNTCTL_CLASS,
			Standard = ICC_STANDARD_CLASSES,
			Cool = ICC_COOL_CLASSES,
			Bar = ICC_BAR_CLASSES
		};
	}

	namespace MessageBoxStyle
	{
		enum
		{
			OK = MB_OK
		};
	}

	namespace ShowWindowOptions
	{
		enum
		{
			Hide = SW_HIDE,
			ShowNormal = SW_SHOWNORMAL
		};
	}

	namespace PeekMessageOptions
	{
		enum
		{
			NoRemove = PM_NOREMOVE,
			Remove = PM_REMOVE,
			NoYield = PM_NOYIELD
		};
	}

	namespace Color
	{
		enum
		{
			ColorWindow = COLOR_WINDOW
		};
	}

	namespace Messages
	{
		enum
		{
			Quit = WM_QUIT,
			Close = WM_CLOSE,
			Destroy = WM_DESTROY,
			Size = WM_SIZE,
			LeftButtonDown = WM_LBUTTONDOWN,
			LeftButtonUp = WM_LBUTTONUP,
			NonClientCreate = WM_NCCREATE,
			Paint = WM_PAINT,
			KeyUp = WM_KEYUP,
			Create = WM_CREATE,
			Command = WM_COMMAND,
			CtlColorBtn = WM_CTLCOLORBTN,
			Notify = WM_NOTIFY,
			CustomDraw = NM_CUSTOMDRAW,
			SetFont = WM_SETFONT,
			ButtonClick = BM_CLICK,
			ButtonClicked = BN_CLICKED,
			DrawItem = WM_DRAWITEM,
			MouseHover = WM_MOUSEHOVER,
			MouseLeave = WM_MOUSELEAVE,
			MouseMove = WM_MOUSEMOVE,
			EraseBackground = WM_ERASEBKGND
		};
	}

	namespace MouseKeys
	{
		enum
		{
			LeftButton = MK_LBUTTON
		};
	}

	namespace VirtualKeyCodes
	{
		enum
		{
			Numpad0 = VK_NUMPAD0,
			Numpad1 = VK_NUMPAD1,
			Numpad2 = VK_NUMPAD2,
			Numpad3 = VK_NUMPAD3,
			Numpad4 = VK_NUMPAD4,
			Numpad5 = VK_NUMPAD5,
			Numpad6 = VK_NUMPAD6,
			Numpad7 = VK_NUMPAD7,
			Numpad8 = VK_NUMPAD8,
			Numpad9 = VK_NUMPAD9,
			Multiply = VK_MULTIPLY,
			Add = VK_ADD,
			Separator = VK_SEPARATOR,
			Subtract = VK_SUBTRACT,
			Decimal = VK_DECIMAL,
			Divide = VK_DIVIDE,
			NumpadEnter = VK_RETURN,
			Delete = VK_DELETE
		};
	}

	namespace FailFast
	{
		enum
		{
			Fatal = FAST_FAIL_FATAL_APP_EXIT
		};
	}

	namespace FormatMessageOptions
	{
		enum
		{
			AllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER,
			FromSystem = FORMAT_MESSAGE_FROM_SYSTEM,
			IgnoreInserts = FORMAT_MESSAGE_IGNORE_INSERTS,
			FromHModule = FORMAT_MESSAGE_FROM_HMODULE
		};
	}

	namespace WindowStyles
	{
		enum : Win32::DWORD
		{
			WsOverlappedWindow = WS_OVERLAPPEDWINDOW,
			WindowEdge = WS_EX_WINDOWEDGE,
			ThickFrame = WS_THICKFRAME
		};
	}

	namespace Styles
	{
		enum
		{
			Child = WS_CHILD,
			ClipSiblings = WS_CLIPSIBLINGS,
			Visible = WS_VISIBLE,
			OverlappedWindow = WS_OVERLAPPEDWINDOW,
			HScroll = WS_HSCROLL,
			VScroll = WS_VSCROLL,
			Border = WS_BORDER,
			DefPushButton = BS_DEFPUSHBUTTON,
			PushButton = BS_PUSHBUTTON,
			VRedraw = CS_VREDRAW,
			HRedraw = CS_HREDRAW,
			AutoCheckBox = BS_AUTOCHECKBOX,
			PushLike = BS_PUSHLIKE,
			ButtonOwnerDrawn = BS_OWNERDRAW,
		};

		namespace Static
		{
			enum
			{
				Right = SS_RIGHT
			};
		}
	}

	constexpr Win32Constant<IDI_APPLICATION> IdiApplication;
	constexpr Win32Constant<IDC_ARROW> IdcArrow;

	constexpr auto CpUtf8 = CP_UTF8;
	constexpr auto WcNoBestFitChars = WC_NO_BEST_FIT_CHARS;

	constexpr auto LoWord(auto value) noexcept -> WORD { return LOWORD(value); }
	constexpr auto HiWord(auto value) noexcept -> WORD { return HIWORD(value); }

	namespace Color
	{
		enum
		{
			Window = COLOR_WINDOW
		};
	}
}
