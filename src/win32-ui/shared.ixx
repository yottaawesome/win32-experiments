module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wingdi.h>
#include <CommCtrl.h>

export module shared;
import std;

export namespace Win32
{
	constexpr auto Sw_ShowDefault = SW_SHOWDEFAULT;
	constexpr auto Icc_Tab_Classes = ICC_TAB_CLASSES;
	constexpr auto Tcif_Text = TCIF_TEXT;
	constexpr auto Tcif_Image = TCIF_IMAGE;
	constexpr auto Cw_UseDefault = CW_USEDEFAULT;
	constexpr auto Cs_HRedraw = CS_HREDRAW;
	constexpr auto Cs_VRedraw = CS_VREDRAW;
	constexpr auto White_Brush = WHITE_BRUSH;
	constexpr auto Swp_ShowWindow = SWP_SHOWWINDOW;
	
	namespace MessageBoxes
	{
		constexpr auto IconExclamation = MB_ICONEXCLAMATION;
		constexpr auto OK = MB_OK;
	}

	using
		::HINSTANCE,
		::HWND,
		::HMENU,
		::LRESULT,
		::UINT,
		::WPARAM,
		::LPARAM,
		::WNDCLASSEX,
		::HBRUSH,
		::MSG,
		::DWORD,
		::LPWSTR,
		::RECT,
		::INITCOMMONCONTROLSEX,
		::TCITEMW,
		::TCHAR,
		::ATOM,
		::HRESULT,
		::UINT_PTR,
		::DWORD_PTR,
		::CREATESTRUCT,
		::LONG_PTR,
		::PAINTSTRUCT,
		::HDC,
		::LPPOINT,
		::COLORREF,
		::LPNMCUSTOMDRAW,
		::HBITMAP,
		::WNDCLASSEXW,
		::LPNMHDR,
		::HPEN,
		::HGDIOBJ,
		::HMODULE,
		::PEXCEPTION_POINTERS,
		::EXCEPTION_POINTERS,
		::WNDPROC,
		::UnregisterClassW,
		::AddVectoredExceptionHandler,
		::__fastfail,
		::FormatMessageA,
		::FormatMessageW,
		::LocalFree,
		::IsDlgButtonChecked,
		::RoundRect,
		::CreatePen,
		::GetSysColorBrush,
		::CreateSolidBrush,
		::LoadCursorW,
		::FillRect,
		::CreatePatternBrush,
		::DeleteObject,
		::DeleteDC,
		::CreateCompatibleDC,
		::CreateCompatibleBitmap,
		::SelectObject,
		::CreateWindowExW,
		::ShowWindow,
		::UpdateWindow,
		::GetModuleHandleW,
		::DefWindowProcW,
		::GetSystemMetrics,
		::GetStockObject,
		::RegisterClassExW,
		::MessageBoxW,
		::GetMessageW,
		::TranslateMessage,
		::DispatchMessageW,
		::PostQuitMessage,
		::DestroyWindow,
		::GetLastError,
		::LoadIconW,
		::LoadImageW,
		::InitCommonControlsEx,
		::GetClientRect,
		::SendMessageW,
		::LoadStringW,
		::SetWindowPos,
		::DefSubclassProc,
		::SetWindowSubclass,
		::SetWindowLongPtrW,
		::GetWindowLongPtrW,
		::FillRect,
		::BeginPaint,
		::EndPaint,
		::MapWindowPoints,
		::GetParent,
		::SetWindowPlacement,
		::SetWindowPos,
		::SetFocus,
		::WideCharToMultiByte,
		::MultiByteToWideChar
		;

	constexpr auto CdisSelected = CDIS_SELECTED;
	constexpr auto CdisHot = CDIS_HOT;
	constexpr auto PsInsideFrame = PS_INSIDEFRAME;

	constexpr auto CpUtf8 = CP_UTF8;
	constexpr auto WcNoBestFitChars = WC_NO_BEST_FIT_CHARS;

	namespace FormatMessageFlags
	{
		constexpr auto AllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER;
		constexpr auto FromSystem = FORMAT_MESSAGE_FROM_SYSTEM;
		constexpr auto IgnoreInserts = FORMAT_MESSAGE_IGNORE_INSERTS;
		constexpr auto FromHModule = FORMAT_MESSAGE_FROM_HMODULE;
	}

	namespace FailFast
	{
		enum
		{
			FatalExit = FAST_FAIL_FATAL_APP_EXIT // FatalAppExit is deffed to something else
		};
	}

	COLORREF GetRGB(DWORD r, DWORD g, DWORD b)
	{
		return RGB(r, g, b);
	}

	BYTE GetRedValue(DWORD rgb)
	{
		return GetRValue(rgb);
	}

	BYTE GetGreenValue(DWORD rgb)
	{
		return GetGValue(rgb);
	}

	BYTE GetBlueValue(DWORD rgb)
	{
		return GetBValue(rgb);
	}

	LPWSTR IdiApplication() noexcept
	{
		return IDI_APPLICATION;
	}

	constexpr auto NoSize = SWP_NOSIZE;

	constexpr auto Gwlp_UserData = GWLP_USERDATA;

	constexpr auto ColorWindow = COLOR_WINDOW;

	auto GetLowWord(LPARAM lparam) noexcept -> WORD
	{
		return LOWORD(lparam);
	}

	auto GetHighWord(LPARAM lparam) noexcept -> WORD
	{
		return HIWORD(lparam);
	}

	auto SplitWord(LPARAM lparam) noexcept -> std::pair<WORD, WORD>
	{
		return { GetLowWord(lparam), GetHighWord(lparam) };
	}

	// TabCtrl_InsertItem
	auto AddTab(HWND hwnd, int iItem, TCITEMW pitem) noexcept -> LRESULT
	{
		return SendMessageW((hwnd), TCM_INSERTITEM, (WPARAM)(int)(iItem), (LPARAM)(const TC_ITEM*)(&pitem));
	}

	namespace ExtendedStyles
	{
		constexpr auto ClientEdge = WS_EX_CLIENTEDGE;
	}

	namespace Styles
	{
		constexpr auto Child = WS_CHILD;
		constexpr auto ClipSiblings = WS_CLIPSIBLINGS;
		constexpr auto Visible = WS_VISIBLE;
		constexpr auto OverlappedWindow = WS_OVERLAPPEDWINDOW;
		constexpr auto HScroll = WS_HSCROLL;
		constexpr auto VScroll = WS_VSCROLL;
		constexpr auto PushButton = BS_DEFPUSHBUTTON;
		constexpr auto VRedraw = CS_VREDRAW;
		constexpr auto HRedraw = CS_HREDRAW;
		constexpr auto AutoCheckBox = BS_AUTOCHECKBOX;
		constexpr auto PushLike = BS_PUSHLIKE;
	}

	constexpr auto DoDefault = CDRF_DODEFAULT;

	namespace Controls
	{
		constexpr auto TabControl = WC_TABCONTROL;
		constexpr auto Button = WC_BUTTON;
	}

	namespace Messages
	{
		constexpr auto Quit = WM_QUIT;
		constexpr auto Close = WM_CLOSE;
		constexpr auto Destroy = WM_DESTROY;
		constexpr auto Size = WM_SIZE;
		constexpr auto LeftButtonUp = WM_LBUTTONUP;
		constexpr auto NonClientCreate = WM_NCCREATE;
		constexpr auto Paint = WM_PAINT;
		constexpr auto KeyUp = WM_KEYUP;
		constexpr auto Create = WM_CREATE;
		constexpr auto Command = WM_COMMAND;
		constexpr auto CtlColorBtn = WM_CTLCOLORBTN;
		constexpr auto Notify = WM_NOTIFY;
		constexpr auto CustomDraw = NM_CUSTOMDRAW;
	}

	// https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
	namespace VirtualKeys
	{
		constexpr auto Up = VK_UP;
		constexpr auto Down = VK_DOWN;
		constexpr auto Left = VK_LEFT;
		constexpr auto Right = VK_RIGHT;
	}

	template<typename R, typename T>
	consteval auto To(T x) noexcept -> R
	{
		return R(x);
	}

	namespace Cursors
	{
		const auto Arrow = IDC_ARROW;
		const auto IBeam = IDC_IBEAM;
		const auto Wait = IDC_WAIT;
		const auto Cross = IDC_CROSS;
		const auto UpArrow = IDC_UPARROW;
		const auto SizeNWSE = IDC_SIZENWSE;
		const auto SizeNESW = IDC_SIZENESW;
		const auto SizeWE = IDC_SIZEWE;
		const auto SizeNS = IDC_SIZENS;
		const auto SizeAll = IDC_SIZEALL;
		const auto No = IDC_NO;
		const auto Hand = IDC_HAND;
		const auto AppStarting = IDC_APPSTARTING;
		const auto Help = IDC_HELP;
		const auto Pin = IDC_PIN;
		const auto Person = IDC_PERSON;
	}

	auto MakeIntResource(int i) noexcept -> LPWSTR
	{
		return MAKEINTRESOURCEW(i);
	}
}
