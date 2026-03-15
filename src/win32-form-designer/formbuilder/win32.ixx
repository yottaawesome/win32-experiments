module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>

export module formbuilder:win32;

export namespace Win32
{
	// === Types ===
	using
		::HWND,
		::HINSTANCE,
		::HMENU,
		::HBRUSH,
		::HCURSOR,
		::HFONT,
		::HGDIOBJ,
		::DWORD,
		::WORD,
		::UINT,
		::INT_PTR,
		::LONG_PTR,
		::BOOL,
		::WPARAM,
		::LPARAM,
		::LRESULT,
		::LPWSTR,
		::MSG,
		::RECT,
		::WNDCLASSEXW,
		::INITCOMMONCONTROLSEX
		;

	// === Functions ===
	using
		::CreateWindowExW,
		::DestroyWindow,
		::DefWindowProcW,
		::RegisterClassExW,
		::ShowWindow,
		::UpdateWindow,
		::GetMessageW,
		::TranslateMessage,
		::DispatchMessageW,
		::PostQuitMessage,
		::SendMessageW,
		::MessageBoxW,
		::GetLastError,
		::GetStockObject,
		::GetWindowLongPtrW,
		::SetWindowLongPtrW,
		::LoadCursorW,
		::AdjustWindowRectEx,
		::InitCommonControlsEx
		;

	// === HIWORD/LOWORD wrappers ===
	auto GetHighWord(WPARAM wParam) noexcept -> WORD { return HIWORD(wParam); }
	auto GetLowWord(WPARAM wParam) noexcept -> WORD { return LOWORD(wParam); }

	// === Window and control styles ===
	namespace Styles
	{
		constexpr auto OverlappedWindow = WS_OVERLAPPEDWINDOW;
		constexpr auto Child = WS_CHILD;
		constexpr auto Visible = WS_VISIBLE;
		constexpr auto Border = WS_BORDER;
		constexpr auto PushButton = BS_DEFPUSHBUTTON;
		constexpr auto AutoCheckBox = BS_AUTOCHECKBOX;
		constexpr auto GroupBox = BS_GROUPBOX;
		constexpr auto EditAutoHScroll = ES_AUTOHSCROLL;
		constexpr auto StaticLeft = SS_LEFT;
		constexpr auto ListBoxStandard = LBS_STANDARD;
		constexpr auto ComboBoxDropDownList = CBS_DROPDOWNLIST;
	}

	// === Class styles ===
	constexpr auto Cs_HRedraw = CS_HREDRAW;
	constexpr auto Cs_VRedraw = CS_VREDRAW;

	// === Messages ===
	namespace Messages
	{
		constexpr auto Command = WM_COMMAND;
		constexpr auto Destroy = WM_DESTROY;
		constexpr auto SetFont = WM_SETFONT;
	}

	// === Notifications ===
	namespace Notifications
	{
		constexpr auto ButtonClicked = BN_CLICKED;
	}

	// === Control class names ===
	namespace Controls
	{
		constexpr auto Button = WC_BUTTON;
		constexpr auto Static = WC_STATIC;
		constexpr auto Edit = WC_EDIT;
		constexpr auto ListBox = WC_LISTBOX;
		constexpr auto ComboBox = WC_COMBOBOX;
	}

	// === System constants ===
	constexpr auto Gwlp_UserData = GWLP_USERDATA;
	constexpr auto Cw_UseDefault = CW_USEDEFAULT;
	constexpr auto Sw_ShowDefault = SW_SHOWDEFAULT;
	constexpr auto DefaultGuiFont = DEFAULT_GUI_FONT;
	constexpr auto ColorWindow = COLOR_WINDOW;

	// === Cursors ===
	namespace Cursors
	{
		const auto Arrow = IDC_ARROW;
	}

	// === Common controls ===
	constexpr auto Icc_StandardClasses = ICC_STANDARD_CLASSES;

	// === MessageBox flags ===
	constexpr auto Mb_Ok = MB_OK;
	constexpr auto Mb_IconInformation = MB_ICONINFORMATION;
	constexpr auto Mb_IconError = MB_ICONERROR;
}
