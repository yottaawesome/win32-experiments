#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.1.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")

export module app;
export import :common;
export import :win32;
export import :toplevelwindow;
export import :mainwindow;

extern "C" auto wWinMain(Win32::HINSTANCE, Win32::HINSTANCE, Win32::LPWSTR, int) -> int
{
    return static_cast<int>(UI::MainWindow{}.Create().Show().MainLoop());
}
