module;

#include <Windows.h>
#include <CommCtrl.h>
#include <Uxtheme.h>

export module progressbar:progresswindow;
import std;
import :status;

export namespace SimpleProgressWindow
{
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
			WNDCLASSEX wcex{
				.cbSize = sizeof(WNDCLASSEX),
				.style = CS_HREDRAW | CS_VREDRAW,
				.lpfnWndProc = StaticWndProc,
				.cbClsExtra = 0,
				.cbWndExtra = 0,
				.hInstance = GetModuleHandle(0),
				.hIcon = LoadIconW(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION)),
				.hCursor = LoadCursor(NULL, IDC_ARROW),
				.hbrBackground = (HBRUSH)COLOR_WINDOW,
				.lpszMenuName = NULL,
				.lpszClassName = L"ProgressWindow",
				.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION))
			};
			
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