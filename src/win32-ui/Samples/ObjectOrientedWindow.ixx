export module Samples:ObjectOrientedWindow;
import std;
import Shared;

export namespace ObjectOrientedWindow // Approach 1
{
    struct BaseWindow
    {
        virtual ~BaseWindow() = default;
        virtual auto GetClassName() -> std::wstring_view = 0;
        virtual auto GetWindowName() -> std::wstring_view = 0;

        static auto __stdcall MainWndProc(Win32::HWND hwnd, Win32::UINT type, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
        {
            auto pThis = static_cast<BaseWindow*>(nullptr);

            if (type == Win32::Messages::NonClientCreate)
            {
                auto pCreate = reinterpret_cast<Win32::CREATESTRUCT*>(lParam);
                pThis = (BaseWindow*)pCreate->lpCreateParams;
                Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData, (Win32::LONG_PTR)pThis);
                pThis->m_window = hwnd;
            }
            else
            {
                pThis = (BaseWindow*)Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData);
            }

            if (pThis)
                return pThis->HandleMessage(type, wParam, lParam);

            return Win32::DefWindowProcW(hwnd, type, wParam, lParam);
        }

        virtual auto HandleMessage(Win32::UINT type, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT = 0;

        void Init()
        {
            m_class =
                [](std::wstring_view className) -> Win32::ATOM
                {
                    auto wcx = Win32::WNDCLASSEX{
                        .cbSize = sizeof(Win32::WNDCLASSEX), // size of structure
                        .style = Win32::Cs_HRedraw | Win32::Cs_VRedraw,   // redraw if size changes
                        .lpfnWndProc = MainWndProc,         // points to window procedure
                        .hInstance = Win32::GetModuleHandleW(nullptr),             // handle to instance
                        .hbrBackground = (Win32::HBRUSH)Win32::GetStockObject(Win32::White_Brush),   // white background brush
                        .lpszClassName = className.data(),  // name of window class
                    };
                    auto returnValue = Win32::RegisterClassExW(&wcx);
                    if (not returnValue)
                    {
                        std::println("Failed registering window class: {}", Win32::GetLastError());
                        std::terminate();
                    }

                    return returnValue;
                }(this->GetClassName());

            m_window =
                [](auto self) -> Win32::HWND
                {
                    auto window = Win32::CreateWindowExW(
                        0,                          // no extended styles
                        self->GetClassName().data(),           // class name
                        self->GetWindowName().data(),          // window name
                        Win32::Styles::OverlappedWindow,   // overlapped window horizontal scroll and vertical scroll bar
                        Win32::Cw_UseDefault,              // default horizontal position
                        Win32::Cw_UseDefault,              // default vertical position
                        Win32::Cw_UseDefault,              // default width
                        Win32::Cw_UseDefault,              // default height
                        nullptr,                    // no parent or owner window
                        nullptr,                    // class menu used
                        Win32::GetModuleHandleW(nullptr),                      // instance handle
                        self                     // no window creation data
                    );
                    if (not window)
                    {
                        std::println("Failed creating window.");
                        std::terminate();
                    }
                    Win32::ShowWindow(window, Win32::Sw_ShowDefault);
                    Win32::UpdateWindow(window);

                    return window;
                }(this);
        }

        Win32::ATOM m_class = 0;
        Win32::HWND m_window = nullptr;
    };

    struct MainWindow : BaseWindow
    {
        MainWindow()
        {
            Init();
        }

        auto GetClassName() -> std::wstring_view override
        {
            return L"ObjectOrientedWindowTest";
        }

        auto GetWindowName() -> std::wstring_view override
        {
            return L"SubclassTestWindow";
        }

    protected:
        auto HandleMessage(Win32::UINT type, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT override
        {
            switch (type)
            {
                case Win32::Messages::Close:
                    Win32::PostQuitMessage(0);
                    return 0;
            }

            return Win32::DefWindowProcW(m_window, type, wParam, lParam);
        }
    };

    auto MessageLoop() -> int
    {
        auto msg = Win32::MSG{};
        while (true)
        {
            auto gotMessage = Win32::GetMessageW(&msg, static_cast<Win32::HWND>(nullptr), 0, 0);
            if (gotMessage == 0) // WM_QUIT received
                break;
            if (gotMessage == -1) // error
                break;

            Win32::TranslateMessage(&msg);
            Win32::DispatchMessageW(&msg);
        }
        return static_cast<int>(msg.wParam);
    }

    auto Run() -> int
    {
        auto window = MainWindow{};
        return MessageLoop();
    }
}
