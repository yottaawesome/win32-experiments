module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>

export module shared;
export import std;

export namespace Win32
{
	constexpr auto Sw_ShowDefault = SW_SHOWDEFAULT;
	constexpr auto Ws_OverlappedWindow = WS_OVERLAPPEDWINDOW;
	constexpr auto Ws_HScroll = WS_HSCROLL;
	constexpr auto Ws_VScroll = WS_VSCROLL;
	constexpr auto Cw_UseDefault = CW_USEDEFAULT;
	constexpr auto Cs_HRedraw = CS_HREDRAW;
	constexpr auto Cs_VRedraw = CS_VREDRAW;
	constexpr auto White_Brush = WHITE_BRUSH;

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
		::CreateWindowExW,
		::ShowWindow,
		::UpdateWindow,
		::GetModuleHandleW,
		::DefWindowProcW,
		::GetSystemMetrics,
		::GetStockObject,
		::RegisterClassExW,
		::GetMessageW,
		::TranslateMessage,
		::DispatchMessageW,
		::PostQuitMessage,
		::DestroyWindow,
		::GetLastError,
		::LoadIconW,
		::LoadImageW
		;

	namespace Messages
	{
		constexpr auto Quit = WM_QUIT;
		constexpr auto Close = WM_CLOSE;
		constexpr auto Destroy = WM_DESTROY;
	}

	template<typename R, typename T>
	consteval R To(T x)
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

export struct SystemCategoryError final : public std::system_error
{
	SystemCategoryError(
		std::string_view msg, 
		const DWORD errorCode = GetLastError()
	) : system_error(std::error_code{ static_cast<int>(errorCode), std::system_category() }, std::string{ msg })
	{}
};