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
import progressbar;

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
