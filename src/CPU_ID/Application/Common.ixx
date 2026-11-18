export module CpuId:Application.Common;
import std;
import Win32;
import :UI;

export namespace Application
{
	// We don't use TopLevelWindowClass as we don't currently want
	// to allow resizing or maximizing the window.
	using MainWindowClass = UI::WindowClass<
		L"CpuViewer", 
		UI::TopLevelWindow, 
		true,
		Win32::WindowStyles::OverlappedWindow 
			& ~Win32::WindowStyles::ThickFrame 
			& ~Win32::WindowStyles::MaximizeBox
			| Win32::WindowStyles::Visible
	>;
}