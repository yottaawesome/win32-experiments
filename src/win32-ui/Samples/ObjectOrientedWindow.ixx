export module Samples:ObjectOrientedWindow;
import std;
import Shared;

export namespace ObjectOrientedWindow // Approach 1
{
    struct BaseWindow
    {
        virtual ~BaseWindow() = default;
        virtual std::wstring_view GetClassName() = 0;
        virtual std::wstring_view GetWindowName() = 0;

        static Win32::LRESULT __stdcall MainWndProc(Win32::HWND hwnd, Win32::UINT type, Win32::WPARAM wParam, Win32::LPARAM lParam)
        {
            BaseWindow* pThis = nullptr;

            if (type == Win32::Messages::NonClientCreate)
            {
                Win32::CREATESTRUCT* pCreate = (Win32::CREATESTRUCT*)lParam;
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

        virtual Win32::LRESULT HandleMessage(Win32::UINT type, Win32::WPARAM wParam, Win32::LPARAM lParam) = 0;

        void Init()
        {
            m_class =
                [](std::wstring_view className)
                {
                    Win32::WNDCLASSEX wcx{
                        .cbSize = sizeof(wcx),              // size of structure
                        .style = Win32::Cs_HRedraw | Win32::Cs_VRedraw,   // redraw if size changes
                        .lpfnWndProc = MainWndProc,         // points to window procedure
                        .hInstance = Win32::GetModuleHandleW(nullptr),             // handle to instance
                        .hbrBackground = (Win32::HBRUSH)Win32::GetStockObject(Win32::White_Brush),   // white background brush
                        .lpszClassName = className.data(),  // name of window class
                    };
                    Win32::ATOM returnValue = Win32::RegisterClassExW(&wcx);
                    if (not returnValue)
                    {
                        std::println("Failed registering window class: {}", Win32::GetLastError());
                        std::terminate();
                    }

                    return returnValue;
                }(this->GetClassName());

            m_window =
                [](auto self)
                {
                    Win32::HWND window = Win32::CreateWindowExW(
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

        std::wstring_view GetClassName() override
        {
            return L"ObjectOrientedWindowTest";
        }

        std::wstring_view GetWindowName() override
        {
            return L"SubclassTestWindow";
        }

    protected:
        Win32::LRESULT HandleMessage(Win32::UINT type, Win32::WPARAM wParam, Win32::LPARAM lParam) override
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
        MainWindow window;
        return MessageLoop();
    }
}
