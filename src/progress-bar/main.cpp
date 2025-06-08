// Adapted from https://gist.github.com/Soju06/e41f5cff43559ca2ab42563586908553
/* ========= Simple Win32 Progress Window =========
 * Language: C/C++ MSVC | ISO C++20 Standard
 * ========= Simple Win32 Progress Window ========= */

#include <Windows.h>
#include <CommCtrl.h>
#include <Uxtheme.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

import std;

namespace SimpleProgressWindow
{
	struct ProgressStatus
	{
		std::wstring Title;
		std::wstring Text;

		int Value;
		int Min;
		int Max;

		bool Marquee;

		ProgressStatus(std::wstring title, std::wstring text, int value = 0, int min = 0, int max = 100, bool marquee = 0)
		{
			Title = title;
			Text = text;
			Value = value;
			Min = min;
			Max = Max;
			Marquee = marquee;
		}
	};

	class ProgressWindow
	{
		HFONT TitleFont;
		HFONT TextFont;
		HFONT ButtonFont;
		HWND TitleHandle;
		HWND TextHandle;
		HWND ProgressHandle;
		HWND FocusHandle;
		HWND CancelButtonHandle;
		bool CustomTheme = false;
		bool UserClose = false;
	public:
		HWND Handle;
		std::function<bool(ProgressWindow* progress)> OnCancel;
		bool Canceled = false;
		bool Closed = false;
		// [ReadOnly]
		ProgressStatus Status;

		inline ProgressWindow(
			std::wstring title,
			ProgressStatus status,
			std::function<bool(ProgressWindow* progress)> onCancel = 0,
			DWORD dwExStyle = 0,
			DWORD dwStyle = WS_CAPTION
		) : Status(status)
		{
			const int SizeWidth = 360;
			const int SizeHeight = 158;
			const int Padding = 12;
			const int TitleX = Padding;
			const int TitleY = Padding;
			const int TitleWidth = SizeWidth - Padding * 2;
			const int TitleHeight = 22;
			const int TextX = Padding;
			const int TextY = TitleY + TitleHeight + Padding;
			const int TextWidth = SizeWidth - Padding * 2;
			const int TextHeight = 16;
			const int ProgressX = Padding;
			const int ProgressY = TextY + TextHeight + Padding + 6;
			const int ProgressWidth = SizeWidth - Padding * 2;
			const int ProgressHeight = 16;
			const int ButtonWidth = 80;
			const int ButtonHeight = 24;
			const int FocusX = 0;
			const int FocusY = 0;
			const int FocusWidth = SizeWidth;
			const int FocusHeight = SizeHeight - ButtonHeight - Padding * 2;
			OnCancel = onCancel;

			// create class
			WNDCLASSEX wcex{};
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.style = CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc = StaticWndProc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = 0;
			wcex.hInstance = GetModuleHandle(0);
			wcex.hIcon = LoadIconW(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
			wcex.lpszMenuName = NULL;
			wcex.lpszClassName = L"ProgressWindow";
			wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
			RegisterClassEx(&wcex);

			// create window
			RECT rect{
				0, 0,
				rect.left + SizeWidth,
				rect.top + SizeHeight
			};
			AdjustWindowRectEx(&rect, dwStyle, false, 0);

			auto sizeWidth = rect.right - rect.left;
			auto sizeHeight = rect.bottom - rect.top;

			Handle = CreateWindowEx(
				dwExStyle,
				wcex.lpszClassName,
				title.c_str(),
				dwStyle,
				(GetSystemMetrics(SM_CXSCREEN) - sizeWidth) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - sizeHeight) / 2,
				sizeWidth,
				sizeHeight,
				0,
				0,
				wcex.hInstance,
				0
			);
			SetWindowLongPtr(Handle, GWLP_USERDATA, (LONG_PTR)this);

			// focus
			FocusHandle = CreateWindowEx(
				0,
				L"Static",
				0,
				WS_CHILD | WS_VISIBLE,
				FocusX,
				FocusY,
				FocusWidth,
				FocusHeight,
				Handle,
				0,
				0,
				0
			);

			// create title
			TitleHandle = CreateWindowEx(
				0,
				L"Static",
				status.Title.c_str(),
				WS_CHILD | WS_VISIBLE,
				TitleX,
				TitleY,
				TitleWidth,
				TitleHeight,
				Handle,
				0,
				0,
				0
			);
			// create title font
			TitleFont = CreateFont(
				TitleHeight,
				0,
				0,
				0,
				FW_NORMAL,
				FALSE,
				FALSE,
				FALSE,
				DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				DEFAULT_PITCH,
				L"Segoe UI"
			);
			// set title font
			SendMessage(TitleHandle, WM_SETFONT, (WPARAM)TitleFont, TRUE);

			// create text
			TextHandle = CreateWindowEx(
				0,
				L"Static",
				status.Text.c_str(),
				WS_CHILD | WS_VISIBLE,
				TextX,
				TextY,
				TextWidth,
				TextHeight,
				Handle,
				0,
				0,
				0
			);
			// create title font
			TextFont = CreateFont(
				TextHeight,
				0,
				0,
				0,
				FW_NORMAL,
				FALSE,
				FALSE,
				FALSE,
				DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				DEFAULT_PITCH,
				L"Segoe UI"
			);
			// set title font
			SendMessage(TextHandle, WM_SETFONT, (WPARAM)TextFont, TRUE);

			// create progress bar
			ProgressHandle = CreateWindowExA(0,
				PROGRESS_CLASSA,
				0,
				WS_CHILD | WS_VISIBLE,
				ProgressX,
				ProgressY,
				ProgressWidth,
				ProgressHeight,
				Handle,
				0,
				0,
				0
			);

			// create button
			ButtonFont = CreateFont(
				ButtonHeight - 8,
				0,
				0,
				0,
				FW_NORMAL,
				FALSE,
				FALSE,
				FALSE,
				DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				DEFAULT_PITCH,
				L"Segoe UI"
			);
			// create cancel button
			CancelButtonHandle = CreateWindowEx(
				0,
				L"Button",
				L"Cancel",
				WS_CHILD | BS_PUSHBUTTON | (OnCancel ? WS_VISIBLE : 0),
				SizeWidth - Padding - ButtonWidth,
				SizeHeight - Padding - ButtonHeight,
				ButtonWidth,
				ButtonHeight,
				Handle,
				(HMENU)IDCANCEL,
				0,
				0
			);
			// set button font
			SendMessage(CancelButtonHandle, WM_SETFONT, (WPARAM)ButtonFont, TRUE);

			SetMarquee(status.Marquee);
			SetValue(status.Value, status.Min, status.Max);

			UpdateWindow(Handle);
			ShowWindow(Handle, SW_SHOW);
		}

		inline void SetTitle(std::wstring value)
		{
			SendMessageW(TitleHandle, WM_SETTEXT, 0, (LPARAM)value.c_str());
			Status.Title = value;
		}

		inline void SetText(std::wstring value)
		{
			SendMessageW(TextHandle, WM_SETTEXT, 0, (LPARAM)value.c_str());
			Status.Text = value;
		}

		inline void SetText(std::wstring title, std::wstring text)
		{
			SetTitle(title);
			SetText(text);
		}

		inline void SetValue(int value)
		{
			SendMessageA(ProgressHandle, PBM_SETPOS, value, 0);
			Status.Value = value;
		}

		inline void SetProgress(int min, int max)
		{
			SendMessageA(ProgressHandle, PBM_SETRANGE, 0, MAKELPARAM(min, max));
			Status.Min = min;
			Status.Max = max;
		}

		inline void SetValue(int value, int min, int max)
		{
			SetProgress(min, max);
			SendMessageA(ProgressHandle, PBM_SETPOS, value, 0);
			Status.Value = value;
		}

		// default speed: 30ms
		inline void SetMarquee(bool value, int speed = 0)
		{
			auto style = GetWindowLongPtr(ProgressHandle, GWL_STYLE);
			if (value) style |= PBS_MARQUEE;
			else style &= ~PBS_MARQUEE;
			SetWindowLongPtr(ProgressHandle, GWL_STYLE, style);
			SendMessage(ProgressHandle, PBM_SETMARQUEE, value, speed);
			Status.Marquee = value;
		}

		inline void SetBackgroundColor(COLORREF color)
		{
			SetProgressCustomTheme();
			SendMessage(ProgressHandle, PBM_SETBKCOLOR, 0, color);
		}

		inline void SetBarColor(COLORREF color)
		{
			SetProgressCustomTheme();
			SendMessage(ProgressHandle, PBM_SETBARCOLOR, 0, color);
		}

		inline void SetStatus(ProgressStatus status)
		{
			// It will have less overhead to update compared to the old value than to update the graphics buffer.
			if (Status.Title != status.Title)
				SetTitle(status.Title);
			if (Status.Text != status.Text)
				SetText(status.Text);
			if (Status.Marquee != status.Marquee)
				SetMarquee(status.Marquee);
			if (Status.Max != status.Max || Status.Min != status.Min)
				SetProgress(status.Min, status.Max);
			if (Status.Value != status.Value)
				SetValue(status.Value);
			UpdateWindow(Handle);
		}

		inline void Show()
		{
			ShowWindow(Handle, SW_SHOW);
		}

		inline void Hide()
		{
			ShowWindow(Handle, SW_HIDE);
		}

		inline void Close()
		{
			Canceled = false;
			UserClose = true;
			PostMessage(Handle, WM_CLOSE, 0, 0);
		}

		inline void Cancel()
		{
			Canceled = true;
			UserClose = false;
			PostMessage(Handle, WM_CLOSE, 0, 0);
		}

		inline bool Join()
		{
			MSG msg{};

			while (!Closed && GetMessageA(&msg, Handle, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessageA(&msg);
			}

			return !Canceled;
		}

		inline ~ProgressWindow()
		{
			Closed = true;
			// destroy font
			DeleteObject(TitleFont);
			DeleteObject(TextFont);
			DeleteObject(ButtonFont);
			// destroy window
			DestroyWindow(Handle);
		}

	private:
		inline void SetProgressCustomTheme()
		{
			if (!CustomTheme)
			{
				SetWindowTheme(ProgressHandle, L"", L"");
				CustomTheme = true;
			}
		}

		LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			switch (message)
			{
			case WM_DESTROY:
				// If you are using a global message loop, modify it for your situation.
				//PostQuitMessage(0);
				Closed = true;
				return 0;
			case WM_COMMAND:
				if (LOWORD(wParam) != IDCANCEL)
					break;
				// goto WM_CLOSE:
			case WM_CLOSE:
				if (UserClose || (OnCancel && OnCancel(this)))
				{
					Canceled = !UserClose;
					Closed = true;
					DestroyWindow(hWnd);
				}
				break;
			case WM_CTLCOLORSTATIC:
				if ((HWND)lParam == TitleHandle)
				{
					SetBkMode((HDC)wParam, TRANSPARENT);
					SetTextColor((HDC)wParam, RGB(0x1a, 0x48, 0xa3));
					return (LRESULT)GetStockObject(NULL_BRUSH);
				}
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			return 0;
		}

		inline static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			ProgressWindow* window = (ProgressWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (window)
				return window->WndProc(hWnd, message, wParam, lParam);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	};
}

int main() 
{
	// create progress window
	SimpleProgressWindow::ProgressWindow progress(
		L"Microsoft Windows",
		SimpleProgressWindow::ProgressStatus(
			L"Please wait while the features are configured",
			L"This might take several minutes.",
			0, 
			0, 
			0, 
			true
		),
		// If this parameter has a function, a window close button and a Cancel button are displayed.
		[=](SimpleProgressWindow::ProgressWindow* window)
		{
			return MessageBox(
				window->Handle,
				L"Are you sure you want to cancel this operation?", 
				L"OMG",
				MB_YESNO | MB_ICONWARNING | MB_TOPMOST
			) == IDYES;
		},
		// You can use WS_EX_TOPMOST in the dxExStyle parameter.
		WS_EX_TOPMOST,
		// If you use WS_SYSMENU for the dxStyle parameter, you can create an exit button on the window.
		// Exit button works the same as Cancel button.
		// Don't forget to put WS_CAPTION when modifying this parameter!!!
		WS_CAPTION | WS_SYSMENU
	);

	std::thread task = std::thread(
		[&]{
			Sleep(2000);
			// Do not use the Status object as a reference variable or change its value!!
			SimpleProgressWindow::ProgressStatus status = progress.Status; // copy
			status.Marquee = false;
			status.Max = 100;
			status.Min = 0;
			status.Value = 0;
			status.Text = L"This might take several minutes.";
			progress.SetStatus(status);

			for (int i = 0; i <= 100; i += 20) {
				progress.SetText(std::format(L"About {} seconds left.", (100 - i) / 20));
				progress.SetValue(i);
				// or
				// status.Value = i;
				// progress.SetStatus(status);
				Sleep(1000);
			}

			progress.SetText(L"Completing...");
			progress.SetMarquee(true);
			Sleep(5000);
			progress.Close();
		});
	task.detach();

	// Returns true if not canceled.
	auto result = progress.Join();
	MessageBox(
		0,
		result ? L"The task ended successfully." : L"The task was canceled.", L"Result",
		MB_OK | (result ? MB_ICONINFORMATION : MB_ICONERROR)
	);

	return 0;
}
