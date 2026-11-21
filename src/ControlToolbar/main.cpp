// Cleaned up version of the code described here: http://pididu.com/wordpress/blog/winapi-example-common-control-toolbar-in-a-pager/
// Converted to last C++ standard, removed MFC, applied AAA idiom,
// made casts explicit, moved variabled to where they are used,
// used explicit typed functions instead of macros, used std::array 
// instead of C-style arrays, added braces around switch statements,
// improved error handling, added comments, removed unused code, and 
// made control flow clearer.

// toolbartest.cpp : minimal app to demo issue
#pragma comment(lib, "comctl32.lib")

#include "windows.h"
#include "commctrl.h"

import std;

// Globals
auto hWndPager = HWND{};
auto hWndToolbar = HWND{};
// Button list for toolbar
// Note that wide strings explicitly used - TEXT("label") messes up dialog box, even
// if compiled as non-unicode
// Arbitrary command 100-104 assigned to buttons, will never be processed
const auto tbButtonsAdd = std::array{
	TBBUTTON{ MAKELONG(HIST_BACK,0), 100, TBSTATE_ENABLED, BTNS_AUTOSIZE,{ 0 }, 0, (INT_PTR)L"one" },
	TBBUTTON{ MAKELONG(HIST_FORWARD,0), 101, TBSTATE_ENABLED, BTNS_AUTOSIZE,{ 0 }, 0, (INT_PTR)L"two" },
	TBBUTTON{ MAKELONG(HIST_VIEWTREE,0), 102, TBSTATE_ENABLED, BTNS_AUTOSIZE,{ 0 }, 0, (INT_PTR)L"three" },
	TBBUTTON{ MAKELONG(HIST_ADDTOFAVORITES,0), 103, TBSTATE_ENABLED, BTNS_AUTOSIZE,{ 0 }, 0, (INT_PTR)L"four" },
	TBBUTTON{ MAKELONG(HIST_FAVORITES,0), 104, TBSTATE_ENABLED, BTNS_AUTOSIZE,{ 0 }, 0, (INT_PTR)L"five" }
};
const auto numButtons = tbButtonsAdd.size();

auto WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	constexpr auto HandleNotify =
		[](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
		{
			auto notificationMessageHeader = reinterpret_cast<LPNMHDR>(lParam);
			switch (notificationMessageHeader->code)
			{
				case PGN_CALCSIZE:
				{
					// If more than one pager in the program
					// may need to check which one sent message
					auto pCS = LPNMPGCALCSIZE{};
					auto tbSize = SIZE{};
					if (SendMessageW(hWndToolbar, TB_GETIDEALSIZE, 0,reinterpret_cast<LPARAM>(&tbSize)))
					{
						pCS = reinterpret_cast<LPNMPGCALCSIZE>(lParam);
						switch (pCS->dwFlag) 
						{
							case PGF_CALCHEIGHT:
							{
								pCS->iHeight = tbSize.cy;
								break;
							}
							case PGF_CALCWIDTH:
							{
								pCS->iWidth = tbSize.cx;
								break;
							}
						}
					}
					return 0;
				}

				case TBN_TOOLBARCHANGE:
				{
					PostMessageW(hWndPager, PGM_RECALCSIZE, 0, 0);
					return 0;
				}

				case TBN_GETBUTTONINFO:
				{
					auto lpTbNotify = reinterpret_cast<LPTBNOTIFY>(lParam);

					// Pass the next button from the array. There is no need to filter out buttons
					// that are already used—they will be ignored.
					if (lpTbNotify->iItem < numButtons) 
					{
						lpTbNotify->tbButton = tbButtonsAdd[lpTbNotify->iItem];
						return 1;
					}
					else 
					{
						return 0;  // No more buttons.
					}
				}

				case TBN_QUERYINSERT:
				{
					return 1;
				}

				case TBN_QUERYDELETE:
				{
					// no need to check the source of the request, as
					// there is only one toolbar in the program,
					// and the answer is always yes
					return 1;
				}

				default:
				{
					return 0;
				}
			}
		};

	switch (msg)
	{
		case WM_NOTIFY:
		{
			return HandleNotify(hwnd, msg, wParam, lParam);
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		default:
		{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
}

auto wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) -> int
try
{
	auto wc = WNDCLASSEXW{ 
		.cbSize = sizeof(WNDCLASSEXW),
		.lpfnWndProc = WndProc,
		.hInstance = GetModuleHandleW(nullptr),
		.hCursor = LoadCursor(0, IDC_ARROW),
		.hbrBackground = GetSysColorBrush(COLOR_3DFACE),
		.lpszClassName = L"MainClass",
	};
	if (not RegisterClassExW(&wc))
		throw std::runtime_error{ "Failed to register window class" };

	auto hWndMain = CreateWindowW(
		wc.lpszClassName, 
		L"ToolbarTest",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		100, 100, 
		400, 300, 
		0, 
		0, 
		GetModuleHandleW(nullptr), 
		0);
	if (not hWndMain)
		throw std::runtime_error{"Failed to create main window"};

	auto ic = INITCOMMONCONTROLSEX{
		.dwSize = sizeof(INITCOMMONCONTROLSEX),
		.dwICC = ICC_BAR_CLASSES | ICC_PAGESCROLLER_CLASS
	};
	if (not InitCommonControlsEx(&ic))
		throw std::runtime_error{"Failed to initialize common controls"};

	// Create Pager.  Arbitrary ID of 343.
	hWndPager = CreateWindowW(
		WC_PAGESCROLLER, 
		nullptr, 
		WS_VISIBLE | WS_CHILD | PGS_DRAGNDROP | PGS_HORZ,
		50, 100, 
		150, 50, 
		hWndMain, 
		reinterpret_cast<HMENU>(343), 
		GetModuleHandleW(nullptr),
		nullptr);
	if (not hWndPager)
		throw std::runtime_error{ "Failed to create pager window" };

	// Create Toolbar.  Arbitrary ID of 231.
	// The parent window is the Pager.
	hWndToolbar = CreateWindowExW(
		0, 
		TOOLBARCLASSNAME, 
		L"Toolbar in Pager Demo", 
		WS_VISIBLE | WS_CHILD | CCS_NORESIZE | CCS_ADJUSTABLE,
		0, 0, 
		200, 50, 
		hWndPager, 
		reinterpret_cast<HMENU>(231), 
		GetModuleHandleW(nullptr),
		nullptr);
	if (not hWndToolbar)
		throw std::runtime_error{ "Failed to create toolbar window" };

	SendMessageW(hWndToolbar, TB_SETBITMAPSIZE, static_cast<WPARAM>(0), (LPARAM)MAKELONG(24, 24));
	SendMessageW(hWndToolbar, TB_LOADIMAGES, static_cast<WPARAM>(IDB_HIST_LARGE_COLOR), reinterpret_cast<LPARAM>(HINST_COMMCTRL));
	SendMessageW(hWndToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
	SendMessageW(hWndToolbar, TB_ADDBUTTONS, numButtons, reinterpret_cast<LPARAM>(tbButtonsAdd.data()));
	SendMessageW(hWndToolbar, TB_AUTOSIZE, 0, 0);

	// Notify the pager that it contains the toolbar
	// This is in addition to making the Pager Window the parent of the Toolbar Window
	// in CreateWindowEx()
	SendMessageW(hWndPager, PGM_SETCHILD, 0, reinterpret_cast<LPARAM>(hWndToolbar));

	auto msg = MSG{};
	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return static_cast<int>(msg.wParam);
}
catch(const std::exception& ex)
{
	MessageBoxA(nullptr, ex.what(), "Error", MB_OK | MB_ICONERROR);
	return -1;
}
