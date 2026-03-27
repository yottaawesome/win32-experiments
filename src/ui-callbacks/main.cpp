import uicallbacks;
import std;

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

auto wWinMain(Win32::HINSTANCE, Win32::HINSTANCE, Win32::LPWSTR, int) -> int
try
{
    Win32::InitCommonControls();
    auto window = UiCallbacks::UI::Window{L"UICallbacks Sample", 800, 600};
    auto button = UiCallbacks::UI::Button{ window, 1, L"Click me", 10, 10, 100, 30 };
    auto subclassButton = UiCallbacks::UI::SubclassButton{ window, 2, L"Click me", 10, 50, 100, 30 };
    button.OnClick = 
        [&window, &button]
        {
            Win32::MessageBoxW(window.Handle(), L"Button clicked!", L"Info", Win32::MessageBoxOptions::Ok | Win32::MessageBoxOptions::Information);
			button.SetText(L"Clicked!");
	    };
    subclassButton.OnClick = 
        [&window]
        {
            Win32::MessageBoxW(window.Handle(), L"Button2 clicked!", L"Info", Win32::MessageBoxOptions::Ok | Win32::MessageBoxOptions::Information);
            return 0;
	    };
    return static_cast<int>(window.DoMessageLoop());
}
catch(const std::exception& ex)
{
    std::println("Exception: {}", ex.what());
    return 1;
}
