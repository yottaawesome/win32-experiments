export module Samples:SubclassControl;
import std;
import Shared;

// https://learn.microsoft.com/en-us/windows/win32/controls/subclassing-overview#subclassing-controls-using-comctl32dll-version-6
export namespace SubclassControl
{
    namespace Messages = Win32::Messages;

    Win32::LRESULT __stdcall MainWndProc(Win32::HWND hwnd, Win32::UINT type, Win32::WPARAM wParam, Win32::LPARAM lParam)
    {
        switch (type)
        {
            case Messages::Close:
                Win32::DestroyWindow(hwnd);
                return 0;
            case Messages::Destroy:
                Win32::PostQuitMessage(0);
                return 0;
            default:
                return Win32::DefWindowProcW(hwnd, type, wParam, lParam);
        }
    }

    Win32::LRESULT __stdcall SubclassProc(
        Win32::HWND hWnd,
        Win32::UINT msg,
        Win32::WPARAM wParam,
        Win32::LPARAM lParam,
        Win32::UINT_PTR uIdSubclass,
        Win32::DWORD_PTR dwRefData
    )
    {
        switch (msg)
        {
            case Win32::Messages::LeftButtonUp:
                std::println("Button up!");
        }
        return Win32::DefSubclassProc(hWnd, msg, wParam, lParam);
    }

    int MessageLoop()
    {
        Win32::MSG msg;
        while (true)
        {
            int gotMessage = Win32::GetMessageW(&msg, static_cast<Win32::HWND>(nullptr), 0, 0);
            if (gotMessage == 0) // WM_QUIT received
                break;
            if (gotMessage == -1) // error
                break;

            Win32::TranslateMessage(&msg);
            Win32::DispatchMessageW(&msg);
        }
        return static_cast<int>(msg.wParam);
    }

    int Run()
    {
        constexpr std::wstring_view ClassName = L"SubclassTest";
        Win32::ATOM Class =
            []()
            {
                Win32::WNDCLASSEX wcx{
                    .cbSize = sizeof(wcx),              // size of structure
                    .style = Win32::Cs_HRedraw | Win32::Cs_VRedraw,   // redraw if size changes
                    .lpfnWndProc = MainWndProc,         // points to window procedure
                    .cbClsExtra = 0,                    // no extra class memory
                    .cbWndExtra = 0,                    // no extra window memory
                    .hInstance = Win32::GetModuleHandleW(nullptr),             // handle to instance
                    .hIcon = nullptr,// LoadIcon(nullptr, IDI_APPLICATION); // predefined app. icon
                    .hCursor = nullptr,// LoadCursor(nullptr, IDC_ARROW);   // predefined arrow
                    .hbrBackground = (Win32::HBRUSH)Win32::GetStockObject(Win32::White_Brush),   // white background brush
                    .lpszClassName = ClassName.data(),  // name of window class
                    .hIconSm = nullptr
                };
                Win32::ATOM returnValue = Win32::RegisterClassExW(&wcx);
                if (not returnValue)
                {
                    std::println("Failed registering window class.");
                    std::terminate();
                }

                return returnValue;
            }();

        constexpr std::wstring_view WindowName = L"SubclassTestWindow";
        Win32::HWND MainWindow =
            []()
            {
                Win32::HWND window = Win32::CreateWindowExW(
                    0,                          // no extended styles
                    ClassName.data(),           // class name
                    WindowName.data(),          // window name
                    Win32::Styles::OverlappedWindow,   // overlapped window horizontal scroll and vertical scroll bar
                    Win32::Cw_UseDefault,              // default horizontal position
                    Win32::Cw_UseDefault,              // default vertical position
                    Win32::Cw_UseDefault,              // default width
                    Win32::Cw_UseDefault,              // default height
                    nullptr,                    // no parent or owner window
                    nullptr,                    // class menu used
                    Win32::GetModuleHandleW(nullptr),                      // instance handle
                    nullptr                     // no window creation data
                );
                if (not window)
                {
                    std::println("Failed creating window.");
                    std::terminate();
                }
                Win32::ShowWindow(window, Win32::Sw_ShowDefault);
                Win32::UpdateWindow(window);

                return window;
            }();

        Win32::HWND Button =
            [](Win32::HWND parent)
            {
                Win32::HWND button = Win32::CreateWindowExW(
                    0,
                    Win32::Controls::Button,
                    L"OK",
                    Win32::Styles::PushButton | Win32::Styles::Child | Win32::Styles::Visible,
                    100,
                    100,
                    100,
                    100,
                    parent,
                    nullptr,
                    Win32::GetModuleHandleW(nullptr),
                    nullptr
                );
                if (not button)
                {
                    std::println("Failed creating button.");
                    std::terminate();
                }
                if (not Win32::SetWindowSubclass(button, SubclassProc, 5, 0))
                {
                    std::println("Failed subclassing button.");
                    std::terminate();
                }

                return button;
            }(MainWindow);

        return MessageLoop();
    }
}
