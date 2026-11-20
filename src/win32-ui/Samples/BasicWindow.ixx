export module Samples:BasicWindow;
import std;
import Shared;

namespace BasicWindow
{
    // Adapted from https://learn.microsoft.com/en-us/windows/win32/controls/create-a-tab-control-in-the-main-window

    constexpr auto ClassName = std::wstring_view{L"MainWClass"};
    constexpr auto WindowName = std::wstring_view{L"Main Window"};

    using namespace Win32;

    auto TabControl = HWND{nullptr};

    auto OnSize(HWND hwndTab, LPARAM lParam) -> bool
    {
        if (not hwndTab)
            return false;
        // Resize the tab control to fit the client are of main window.
        if (not SetWindowPos(hwndTab, 0, 0, 0, GetLowWord(lParam), GetHighWord(lParam), Swp_ShowWindow))
            return false;
        return true;
    }

    auto MainWndProc(HWND hwnd, UINT type, WPARAM wParam, LPARAM lParam) -> LRESULT
    {
        switch (type)
        {
            case Messages::Close:
                DestroyWindow(hwnd);
                return 0;
            case Messages::Destroy:
                PostQuitMessage(0);
                return 0;
            case Messages::Size:
                OnSize(TabControl, lParam);
                return 0;
            default:
                return DefWindowProcW(hwnd, type, wParam, lParam);
        }
    }

    auto CreateTabControl(HWND hwndParent, HINSTANCE g_hInst) -> HWND
    {
        constexpr auto daysInWeek = 7;

        // Initialize common controls.
        auto icex = INITCOMMONCONTROLSEX{
            .dwSize = sizeof(INITCOMMONCONTROLSEX),
            .dwICC = Icc_Tab_Classes
        };
        InitCommonControlsEx(&icex);

        // Get the dimensions of the parent window's client area, and
        // create a tab control child window of that size. Note that g_hInst
        // is the global instance handle.
        auto rcClient = RECT{};
        GetClientRect(hwndParent, &rcClient);
        auto hwndTab = Win32::CreateWindowExW(
            0,
            Controls::TabControl,
            L"",
            Styles::Child | Styles::ClipSiblings | Styles::Visible,
            0,
            0,
            rcClient.right,
            rcClient.bottom,
            hwndParent,
            nullptr,
            g_hInst,
            nullptr
        );
        if (not hwndTab)
            return nullptr;

        // Add tabs for each day of the week.
        auto tie = TCITEMW{
            .mask = Tcif_Text | Tcif_Image,
            .iImage = -1
        };

        auto name = std::wstring{};
        for (int i = 0; i < daysInWeek; i++)
        {
            switch (i)
            {
                case 0:
                    name = L"Monday";
                    break;
                case 1:
                    name = L"Tuesday";
                    break;
                case 2:
                    name = L"Wednesday";
                    break;
                case 3:
                    name = L"Thursday";
                    break;
                case 4:
                    name = L"Friday";
                    break;
                case 5:
                    name = L"Saturday";
                    break;
                case 6:
                    name = L"Sunday";
                    break;
            }
            tie.pszText = name.data();

            if (AddTab(hwndTab, i, tie) == -1)
            {
                DestroyWindow(hwndTab);
                return nullptr;
            }
        }
        return hwndTab;
    }

    auto InitApplication(HINSTANCE hinstance, std::wstring_view className) -> ATOM
    {
        auto wcx = WNDCLASSEX{
            .cbSize = sizeof(WNDCLASSEX),              // size of structure
            .style = Cs_HRedraw | Cs_VRedraw,   // redraw if size changes
            .lpfnWndProc = MainWndProc,         // points to window procedure
            .cbClsExtra = 0,                    // no extra class memory
            .cbWndExtra = 0,                    // no extra window memory
            .hInstance = hinstance,             // handle to instance
            .hIcon = nullptr,// LoadIcon(nullptr, IDI_APPLICATION); // predefined app. icon
            .hCursor = nullptr,// LoadCursor(nullptr, IDC_ARROW);   // predefined arrow
            .hbrBackground = (HBRUSH)GetStockObject(White_Brush),   // white background brush
            //.lpszMenuName = ClassName.data(), // name of menu resource
            .lpszClassName = className.data(),  // name of window class
            .hIconSm = nullptr
        };

        return RegisterClassExW(&wcx);
    }

    auto CreateButton(HWND parent) -> HWND
    {
        auto button = CreateWindowExW(
            0,
            Controls::Button,
            L"OK",
            Styles::PushButton | Styles::Child | Styles::Visible,
            100,
            100,
            100,
            100,
            parent,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr
        );
        return button;
    }

    auto CreateMainWindow(HINSTANCE hInst, std::wstring_view className, std::wstring_view windowName) -> HWND
    {
        auto hwndMain = CreateWindowExW(
            0,                          // no extended styles
            className.data(),           // class name
            windowName.data(),          // window name
            Styles::OverlappedWindow,   // overlapped window horizontal scroll and vertical scroll bar
            Cw_UseDefault,              // default horizontal position
            Cw_UseDefault,              // default vertical position
            Cw_UseDefault,              // default width
            Cw_UseDefault,              // default height
            nullptr,                    // no parent or owner window
            nullptr,                    // class menu used
            hInst,                      // instance handle
            nullptr                     // no window creation data
        );
        return hwndMain;
    }

	export auto Run() -> int
	{
        auto hinst = GetModuleHandleW(nullptr);

        if (not InitApplication(hinst, ClassName))
            return 1;

        auto hwndMain = CreateMainWindow(hinst, ClassName, WindowName);
        if (not hwndMain)
            return 1;

        TabControl = CreateTabControl(hwndMain, hinst);
        if (not TabControl)
            return 1;

        auto button = CreateButton(hwndMain);

        ShowWindow(hwndMain, Sw_ShowDefault);
        UpdateWindow(hwndMain);

        auto msg = MSG{};
        while (true)
        {
            int fGotMessage = GetMessageW(&msg, (HWND)nullptr, 0, 0);
            if (fGotMessage == 0) // WM_QUIT received
                break;
            if (fGotMessage == -1) // error
                break;

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        return static_cast<int>(msg.wParam);
	}
}
