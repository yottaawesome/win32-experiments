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
	
	namespace Messages
	{
		constexpr auto Quit = WM_QUIT;
		constexpr auto Close = WM_CLOSE;
		constexpr auto Destroy = WM_DESTROY;
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
		::GetLastError
		;
}

export struct SystemCategoryError final : public std::system_error
{
	SystemCategoryError(std::string_view msg, DWORD errorCode = GetLastError())
		: system_error(std::error_code{ static_cast<int>(errorCode), std::system_category() }, std::string{ msg })
	{}
};