export module samples;
import shared;
import util;
export import :more;

#pragma comment(lib, "Comctl32.lib")
// See https://stackoverflow.com/questions/4308503/how-to-enable-visual-styles-without-a-manifest
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace BasicWindow
{
    // Adapted from https://learn.microsoft.com/en-us/windows/win32/controls/create-a-tab-control-in-the-main-window

    constexpr std::wstring_view ClassName = L"MainWClass";
    constexpr std::wstring_view WindowName = L"Main Window";

    using namespace Win32;

    HWND TabControl = nullptr;

    bool OnSize(HWND hwndTab, LPARAM lParam)
    {
        if (not hwndTab)
            return false;

        // Resize the tab control to fit the client are of main window.
        if (!SetWindowPos(hwndTab, 0, 0, 0, GetLowWord(lParam), GetHighWord(lParam), Swp_ShowWindow))
            return false;
        return true;
    }

    LRESULT __stdcall MainWndProc(HWND hwnd, UINT type, WPARAM wParam, LPARAM lParam)
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

    HWND CreateTabControl(HWND hwndParent, HINSTANCE g_hInst)
    {
        constexpr auto daysInWeek = 7;

        // Initialize common controls.
        INITCOMMONCONTROLSEX icex{
            .dwSize = sizeof(INITCOMMONCONTROLSEX),
            .dwICC = Icc_Tab_Classes
        };
        InitCommonControlsEx(&icex);

        // Get the dimensions of the parent window's client area, and 
        // create a tab control child window of that size. Note that g_hInst
        // is the global instance handle.
        RECT rcClient;
        GetClientRect(hwndParent, &rcClient);
        HWND hwndTab = Win32::CreateWindowExW(
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
        TCITEMW tie{
            .mask = Tcif_Text | Tcif_Image,
            .iImage = -1
        };
        
        std::wstring name;
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

    ATOM InitApplication(HINSTANCE hinstance, std::wstring_view className)
    {
        WNDCLASSEX wcx{
            .cbSize = sizeof(wcx),              // size of structure 
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

    HWND CreateButton(HWND parent)
    {
        HWND button = CreateWindowExW(
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

    HWND CreateMainWindow(HINSTANCE hInst, std::wstring_view className, std::wstring_view windowName)
    {
        HWND hwndMain = CreateWindowExW(
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

	export int Run()
	{
        HINSTANCE hinst = GetModuleHandleW(nullptr);

        if (not InitApplication(hinst, ClassName))
            return 1;

        HWND hwndMain = CreateMainWindow(hinst, ClassName, WindowName);
        if (not hwndMain)
            return 1;

        TabControl = CreateTabControl(hwndMain, hinst);
        if (not TabControl)
            return 1;

        HWND button = CreateButton(hwndMain);

        ShowWindow(hwndMain, Sw_ShowDefault);
        UpdateWindow(hwndMain);

        MSG msg;
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

// https://learn.microsoft.com/en-us/windows/win32/controls/subclassing-overview#subclassing-controls-using-comctl32dll-version-6
export namespace SubclassControl
{
    namespace Messages = Win32::Messages;

    Win32::LRESULT __stdcall MainWndProc(HWND hwnd, UINT type, WPARAM wParam, LPARAM lParam)
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
                ATOM returnValue = Win32::RegisterClassExW(&wcx);
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
            [](HWND parent)
            {
                Win32::HWND button = CreateWindowExW(
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
                    GetModuleHandleW(nullptr),
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

export namespace ObjectOrientedWindow // Approach 1
{
    struct BaseWindow
    {
        virtual ~BaseWindow() = default;
        virtual std::wstring_view GetClassName() = 0;
        virtual std::wstring_view GetWindowName() = 0;

        static Win32::LRESULT __stdcall MainWndProc(HWND hwnd, UINT type, WPARAM wParam, LPARAM lParam)
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

        virtual Win32::LRESULT HandleMessage(UINT type, WPARAM wParam, LPARAM lParam) = 0;

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
                    ATOM returnValue = Win32::RegisterClassExW(&wcx);
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
        Win32::LRESULT HandleMessage(UINT type, WPARAM wParam, LPARAM lParam) override
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

export namespace ObjectOrientedWindowTemplate // Approach 2
{
    export template <typename TDerived>
    struct BaseWindow
    {
        // MSVC has a bug
        //static constexpr std::wstring_view x = TDerived::ClassName;
        virtual void Create(
            std::wstring_view lpWindowName,
            Win32::DWORD dwStyle,
            Win32::DWORD dwExStyle = 0,
            int x = Win32::Cw_UseDefault,
            int y = Win32::Cw_UseDefault,
            int nWidth = Win32::Cw_UseDefault,
            int nHeight = Win32::Cw_UseDefault,
            Win32::HWND hWndParent = 0,
            Win32::HMENU hMenu = 0
        )
        {
            Win32::WNDCLASSEX wc{
                .cbSize = sizeof(wc),
                .lpfnWndProc = WindowProc,
                .hInstance = Win32::GetModuleHandleW(nullptr),
                .lpszClassName = TDerived::ClassName.data()
            };
            if (not Win32::RegisterClassExW(&wc))
            {
                std::println("Failed registering class: {}", Win32::GetLastError());
                std::terminate();
            }

            m_hwnd = Win32::CreateWindowExW(
                dwExStyle,
                TDerived::ClassName.data(),
                lpWindowName.data(),
                dwStyle,
                x,
                y,
                nWidth,
                nHeight,
                hWndParent,
                hMenu,
                Win32::GetModuleHandleW(nullptr),
                this
            );
            if (not m_hwnd)
            {
                std::println("Failed creating window: {}", Win32::GetLastError());
                std::terminate();
            }

            Win32::ShowWindow(m_hwnd, Win32::Sw_ShowDefault);
            Win32::UpdateWindow(m_hwnd);
        }

        virtual Win32::HWND Window() const noexcept final { return m_hwnd; }

    protected:
        virtual Win32::LRESULT HandleMessage(Win32::UINT uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) = 0;

        static Win32::LRESULT __stdcall WindowProc(Win32::HWND hwnd, Win32::UINT uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam)
        {
            TDerived* pThis = nullptr;

            if (uMsg == Win32::Messages::NonClientCreate)
            {
                Win32::CREATESTRUCT* pCreate = (Win32::CREATESTRUCT*)lParam;
                pThis = (TDerived*)pCreate->lpCreateParams;
                Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData, (Win32::LONG_PTR)pThis);

                pThis->m_hwnd = hwnd;
            }
            else
            {
                pThis = (TDerived*)GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData);
            }

            if (pThis)
            {
                return pThis->HandleMessage(uMsg, wParam, lParam);
            }
            return Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }

        Win32::HWND m_hwnd = nullptr;
    };

    struct MainWindow final : BaseWindow<MainWindow>
    {
        static constexpr std::wstring_view ClassName = L"Sample Window Class";

        Win32::LRESULT HandleMessage(Win32::UINT uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) override
        {
            switch (uMsg)
            {
                case Win32::Messages::Destroy:
                {
                    Win32::PostQuitMessage(0);
                    return 0;
                }

                default:
                    return Win32::DefWindowProcW(m_hwnd, uMsg, wParam, lParam);
            }
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
        window.Create(L"Blah", Win32::Styles::OverlappedWindow);
        return MessageLoop();
    }
}

export namespace ObjectOrientedControl
{
    template<typename T>
    concept ControlLike = requires (T t)
    {
        t.Handle = (Win32::HWND)0;
        { T::Class.data() } -> std::same_as<const wchar_t*>;
        {t.Text.data() } -> std::same_as<const wchar_t*>;
        t.Styles = Win32::DWORD{ 0 };
    };

    template<ControlLike TType>
    struct Control
    {
        Control(Win32::HWND parent, const Win32::DWORD x, const Win32::DWORD y, const Win32::DWORD width, const Win32::DWORD height)
        {
            Init(parent, x, y, width, height);
        }

        Control(Win32::HWND parent)
        {
            Init(parent, m_control.X, m_control.Y, m_control.Width, m_control.Height);
        }

    protected:
        void Init(Win32::HWND parent, const Win32::DWORD x, const Win32::DWORD y, const Win32::DWORD width, const Win32::DWORD height)
        {
            m_control.Handle = CreateWindowExW(
                0,
                TType::Class.data(),
                m_control.Text.data(),
                m_control.Styles,
                x,
                y,
                width,
                height,
                parent,
                (Win32::HMENU)(Win32::UINT_PTR)(m_control.Id),
                GetModuleHandleW(nullptr),
                nullptr
            );
            if (not m_control.Handle)
                throw Error::Win32Error(Win32::GetLastError(), "Failed creating button.");
            if (not Win32::SetWindowSubclass(m_control.Handle, SubclassProc, 5, reinterpret_cast<Win32::DWORD_PTR>(this)))
            {
                std::println("Failed subclassing button.");
                std::terminate();
            }
        }

        static Win32::LRESULT __stdcall SubclassProc(
            Win32::HWND hwnd,
            Win32::UINT msg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam,
            Win32::UINT_PTR idSubclass,
            Win32::DWORD_PTR refData
        )
        {
            Control<TType>* pThis = reinterpret_cast<Control<TType>*>(refData);
            return pThis 
                ? pThis->m_control.Process(msg, wParam, lParam, idSubclass, refData)
                : Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
        }

        protected:
        TType m_control;
    };

    template<typename T>
    concept S = requires(T t) { t.Blah(); };

    template<
        Util::FixedString VClassName,
        Win32::DWORD VStyles,
        Util::FixedString VText = L"",
        Win32::DWORD VControlId = 0,
        Win32::DWORD VX = 100,
        Win32::DWORD VY = 100,
        Win32::DWORD VWidth = 100,
        Win32::DWORD VHeight = 100
    >
    struct ControlTraits
    {
        virtual ~ControlTraits() = default;
        static constexpr std::wstring_view Class = VClassName.View();
        Win32::DWORD Styles = VStyles;
        std::wstring_view Text = VText.View();
        Win32::DWORD Id = VControlId;
        Win32::DWORD X = VX;
        Win32::DWORD Y = VY;
        Win32::DWORD Width = VWidth;
        Win32::DWORD Height = VHeight;
        Win32::HWND Handle = nullptr;

        void Reposition(const long deltaLeft, const long deltaTop)
        {
            // https://stackoverflow.com/questions/18034975/how-do-i-find-position-of-a-win32-control-window-relative-to-its-parent-window
            Win32::RECT rc;
            Win32::GetClientRect(Handle, &rc);
            Win32::MapWindowPoints(Handle, Win32::GetParent(Handle), (Win32::LPPOINT)&rc, 2);

            rc.left += deltaLeft;
            rc.top += deltaTop;

            Win32::SetWindowPos(Handle, nullptr, rc.left, rc.top, 0, 0, Win32::NoSize);
        }

        virtual Win32::LRESULT Process(
            Win32::UINT msg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam,
            Win32::UINT_PTR uIdSubclass,
            Win32::DWORD_PTR dwRefData
        )
        {
            return Win32::DefSubclassProc(this->Handle, msg, wParam, lParam);
        }
    };

    template<
        Util::FixedString VText,
        Win32::DWORD VId,
        Win32::DWORD VX,
        Win32::DWORD VY,
        Win32::DWORD VWidth,
        Win32::DWORD VHeight
    >
    struct TestButtonTraits 
        //: public ButtonTraits<VText, VId, VX, VY, VWidth, VHeight>
        : public ControlTraits<L"Button", Win32::Styles::PushButton | Win32::Styles::Child | Win32::Styles::Visible, VText, VId, VX, VY, VWidth, VHeight>
    {
        void HandleKeyDown(Win32::WPARAM key)
        {
            switch (key)
            {
                case Win32::VirtualKeys::Up:
                {
                    this->Reposition(0, -10);
                    break;
                }

                case Win32::VirtualKeys::Right:
                {
                    this->Reposition(10, 0);
                    break;
                }

                case Win32::VirtualKeys::Down:
                {
                    this->Reposition(0, 10);
                    break;
                }

                case Win32::VirtualKeys::Left:
                {
                    this->Reposition(-10, 0);
                    break;
                }
            }
        }

        Win32::LRESULT Process(
            Win32::UINT msg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam,
            Win32::UINT_PTR uIdSubclass,
            Win32::DWORD_PTR dwRefData
        ) override
        {
            switch (msg)
            {
                case Win32::Messages::LeftButtonUp:
                {
                    auto re = Win32::SetFocus(this->Handle);
                    std::println("Left mouse button up!");
                    break;
                }
                    
                // https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-keyup
                case Win32::Messages::KeyUp:
                {
                    HandleKeyDown(wParam);
                    break;
                }
            }
            return Win32::DefSubclassProc(this->Handle, msg, wParam, lParam);
        }
    };

    using ButtonA = Control<TestButtonTraits<L"Hello, world!", 10, 100, 100, 100, 100>>;
    using ButtonB = Control<TestButtonTraits<L"Hello again!!", 10, 250, 100, 100, 100>>;

    template<typename TType>
    class Window
    {
        TType m_window = Init();

        HWND Init()
        {
            Win32::WNDCLASSEX wc{
                .cbSize = sizeof(wc),
                .lpfnWndProc = WindowProc,
                .hInstance = Win32::GetModuleHandleW(nullptr),
                .hbrBackground = (Win32::HBRUSH)Win32::GetStockObject(Win32::White_Brush),   // white background brush 
                .lpszClassName = TType::ClassName.data()
            };
            if (not Win32::RegisterClassExW(&wc))
                throw Error::Win32Error(Win32::GetLastError(), "Failed registering class");

            Win32::HWND hwnd = Win32::CreateWindowExW(
                0,
                TType::ClassName.data(),
                TType::WindowName.data(),
                Win32::Styles::OverlappedWindow,
                Win32::Cw_UseDefault,
                Win32::Cw_UseDefault,
                Win32::Cw_UseDefault,
                Win32::Cw_UseDefault,
                nullptr,
                nullptr,
                Win32::GetModuleHandleW(nullptr),
                this
            );
            if (not hwnd)
                throw Error::Win32Error(Win32::GetLastError(), "Failed creating window");

            Win32::ShowWindow(hwnd, Win32::Sw_ShowDefault);
            Win32::UpdateWindow(hwnd);
            return hwnd;
        }

        static Win32::LRESULT __stdcall WindowProc(Win32::HWND hwnd, Win32::UINT uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam)
        {
            Window<TType>* pThis = nullptr;

            if (uMsg == Win32::Messages::NonClientCreate)
            {
                Win32::CREATESTRUCT* pCreate = (Win32::CREATESTRUCT*)lParam;
                pThis = (Window<TType>*)pCreate->lpCreateParams;
                Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData, (Win32::LONG_PTR)pThis);

                pThis->m_window.WindowHandle = hwnd;
            }
            else
            {
                pThis = (Window<TType>*)GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData);
            }

            return pThis
                ? pThis->m_window.HandleMessage(uMsg, wParam, lParam)
                : Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }
    };

    struct MainWindowType
    {
        MainWindowType(HWND handle) : WindowHandle(handle) {}

        static constexpr std::wstring_view ClassName = L"MainWindow";
        static constexpr std::wstring_view WindowName = L"MainWindow";

        HWND WindowHandle = nullptr;
        ButtonA m_buttonA{ WindowHandle };
        ButtonB m_buttonB{ WindowHandle };

        Win32::LRESULT HandleMessage(Win32::UINT uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam)
        {
            switch (uMsg)
            {
                case Win32::Messages::Destroy:
                {
                    Win32::PostQuitMessage(0);
                    return 0;
                }

                default:
                    return Win32::DefWindowProcW(WindowHandle, uMsg, wParam, lParam);
            }
        }
    };

    using MainWindow = Window<MainWindowType>;

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

    int Run() try
    {
        MainWindow m;
        return MessageLoop();
    }
    catch (const std::exception& ex)
    {
        std::println("{}", ex.what());
        return 1;
    }
}

namespace Gradient
{
    // https://stackoverflow.com/questions/18745447/how-can-i-change-the-background-color-of-a-button-winapi-c
    constexpr auto IDC_EXIT_BUTTON = 101;
    constexpr auto IDC_PUSHLIKE_BUTTON = 102;

    Win32::HBRUSH CreateGradientBrush(Win32::COLORREF top, Win32::COLORREF bottom, Win32::LPNMCUSTOMDRAW item)
    {
        Win32::HBRUSH Brush = nullptr;
        Win32::HDC hdcmem = Win32::CreateCompatibleDC(item->hdc);
        Win32::HBITMAP hbitmap = Win32::CreateCompatibleBitmap(item->hdc, item->rc.right - item->rc.left, item->rc.bottom - item->rc.top);
        Win32::SelectObject(hdcmem, hbitmap);

        int r1 = Win32::GetRedValue(top), 
            r2 = Win32::GetRedValue(bottom), 
            g1 = Win32::GetGreenValue(top), 
            g2 = Win32::GetGreenValue(bottom), 
            b1 = Win32::GetBlueValue(top), 
            b2 = Win32::GetBlueValue(bottom);
        for (int i = 0; i < item->rc.bottom - item->rc.top; i++)
        {
            Win32::RECT temp;
            int r, g, b;
            r = int(r1 + double(i * (r2 - r1) / item->rc.bottom - item->rc.top));
            g = int(g1 + double(i * (g2 - g1) / item->rc.bottom - item->rc.top));
            b = int(b1 + double(i * (b2 - b1) / item->rc.bottom - item->rc.top));
            Brush = Win32::CreateSolidBrush(Win32::GetRGB(r, g, b));
            temp.left = 0;
            temp.top = i;
            temp.right = item->rc.right - item->rc.left;
            temp.bottom = i + 1;
            Win32::FillRect(hdcmem, &temp, Brush);
            Win32::DeleteObject(Brush);
        }
        Win32::HBRUSH pattern = Win32::CreatePatternBrush(hbitmap);

        Win32::DeleteDC(hdcmem);
        Win32::DeleteObject(Brush);
        Win32::DeleteObject(hbitmap);

        return pattern;
    }

    void CreateButtons(Win32::HWND hwnd)
    {
        Win32::HWND Exit_Button = Win32::CreateWindowExW(
            0,
            L"BUTTON",
            L"EXIT",
            Win32::Styles::Visible | Win32::Styles::Child | Win32::Styles::PushButton,
            50,
            50,
            100,
            100,
            hwnd,
            (Win32::HMENU)IDC_EXIT_BUTTON,
            nullptr,
            nullptr
        );
        if (Exit_Button == nullptr)
        {
            Win32::MessageBoxW(nullptr, L"Button Creation Failed!", L"Error!", Win32::MessageBoxes::IconExclamation);
            std::terminate();
        }

        Win32::HWND Pushlike_Button = Win32::CreateWindowExW(
            0,
            L"BUTTON",
            L"PUSH ME!",
            Win32::Styles::Visible | Win32::Styles::Child | Win32::Styles::AutoCheckBox | Win32::Styles::PushLike,
            200,
            50,
            100,
            100,
            hwnd,
            (Win32::HMENU)IDC_PUSHLIKE_BUTTON,
            nullptr,
            nullptr
        );
        if (Pushlike_Button == nullptr)
        {
            Win32::MessageBoxW(nullptr, L"Button Creation Failed!", L"Error!", Win32::MessageBoxes::IconExclamation);
            std::terminate();
        }
    }

    Win32::LRESULT __stdcall MainWindow(Win32::HWND hwnd, Win32::UINT msg, Win32::WPARAM wParam, Win32::LPARAM lParam)
    {
        static HBRUSH defaultbrush = nullptr;
        static HBRUSH hotbrush = nullptr;
        static HBRUSH selectbrush = nullptr;
        static HBRUSH push_uncheckedbrush = nullptr;
        static HBRUSH push_checkedbrush = nullptr;
        static HBRUSH push_hotbrush1 = nullptr;
        static HBRUSH push_hotbrush2 = nullptr;

        switch (msg)
        {
            case Win32::Messages::Create:
            {
                CreateButtons(hwnd);
                break;
            }

            case Win32::Messages::Command:
            {
                switch (Win32::GetLowWord(wParam))
                {
                    case IDC_EXIT_BUTTON:
                    {
                        Win32::SendMessageW(hwnd, Win32::Messages::Close, 0, 0);
                    }
                    break;
                }
                break;
            }

            case Win32::Messages::CtlColorBtn: //In order to make those edges invisble when we use RoundRect(),
            {                //we make the color of our button's background match window's background
                return (Win32::LRESULT)Win32::GetSysColorBrush(Win32::ColorWindow + 1);
            }

            case Win32::Messages::Notify:
            {
                Win32::LPNMHDR some_item = (Win32::LPNMHDR)lParam;

                if (some_item->idFrom == IDC_EXIT_BUTTON && some_item->code == Win32::Messages::CustomDraw)
                {
                    Win32::LPNMCUSTOMDRAW item = (Win32::LPNMCUSTOMDRAW)some_item;

                    if (item->uItemState & Win32::CdisSelected)
                    {
                        //Select our color when the button is selected
                        if (selectbrush == nullptr)
                            selectbrush = CreateGradientBrush(Win32::GetRGB(180, 0, 0), Win32::GetRGB(255, 180, 0), item);

                        //Create pen for button border
                        Win32::HPEN pen = Win32::CreatePen(Win32::PsInsideFrame, 0, Win32::GetRGB(0, 0, 0));

                        //Select our brush into hDC
                        Win32::HGDIOBJ old_pen = Win32::SelectObject(item->hdc, pen);
                        Win32::HGDIOBJ old_brush = Win32::SelectObject(item->hdc, selectbrush);

                        //If you want rounded button, then use this, otherwise use FillRect().
                        Win32::RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 5, 5);

                        //Clean up
                        Win32::SelectObject(item->hdc, old_pen);
                        Win32::SelectObject(item->hdc, old_brush);
                        Win32::DeleteObject(pen);

                        //Now, I don't want to do anything else myself (draw text) so I use this value for return:
                        return Win32::DoDefault;
                        //Let's say I wanted to draw text and stuff, then I would have to do it before return with
                        //DrawText() or other function and return CDRF_SKIPDEFAULT
                    }
                    else
                    {
                        if (item->uItemState & Win32::CdisHot) //Our mouse is over the button
                        {
                            //Select our color when the mouse hovers our button
                            if (hotbrush == nullptr)
                                hotbrush = CreateGradientBrush(Win32::GetRGB(255, 230, 0), Win32::GetRGB(245, 0, 0), item);

                            Win32::HPEN pen = Win32::CreatePen(Win32::PsInsideFrame, 0, Win32::GetRGB(0, 0, 0));

                            Win32::HGDIOBJ old_pen = Win32::SelectObject(item->hdc, pen);
                            Win32::HGDIOBJ old_brush = Win32::SelectObject(item->hdc, hotbrush);

                            Win32::RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 5, 5);

                            Win32::SelectObject(item->hdc, old_pen);
                            Win32::SelectObject(item->hdc, old_brush);
                            Win32::DeleteObject(pen);

                            return Win32::DoDefault;
                        }

                        //Select our color when our button is doing nothing
                        if (defaultbrush == nullptr)
                            defaultbrush = CreateGradientBrush(Win32::GetRGB(255, 180, 0), Win32::GetRGB(180, 0, 0), item);

                        Win32::HPEN pen = Win32::CreatePen(Win32::PsInsideFrame, 0, Win32::GetRGB(0, 0, 0));

                        Win32::HGDIOBJ old_pen = Win32::SelectObject(item->hdc, pen);
                        Win32::HGDIOBJ old_brush = Win32::SelectObject(item->hdc, defaultbrush);

                        Win32::RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 5, 5);

                        Win32::SelectObject(item->hdc, old_pen);
                        Win32::SelectObject(item->hdc, old_brush);
                        Win32::DeleteObject(pen);

                        return Win32::DoDefault;
                    }
                }
                else if (some_item->idFrom == IDC_PUSHLIKE_BUTTON && some_item->code == Win32::Messages::CustomDraw)
                {
                    Win32::LPNMCUSTOMDRAW item = (Win32::LPNMCUSTOMDRAW)some_item;

                    if (Win32::IsDlgButtonChecked(hwnd, some_item->idFrom))
                    {
                        if (item->uItemState & Win32::CdisHot)
                        {

                            if (push_hotbrush1 == nullptr)
                                push_hotbrush1 = CreateGradientBrush(Win32::GetRGB(0, 0, 245), Win32::GetRGB(0, 230, 255), item);

                            Win32::HPEN pen = CreatePen(Win32::PsInsideFrame, 0, Win32::GetRGB(0, 0, 0));

                            Win32::HGDIOBJ old_pen = SelectObject(item->hdc, pen);
                            Win32::HGDIOBJ old_brush = SelectObject(item->hdc, push_hotbrush1);

                            Win32::RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 10, 10);

                            Win32::SelectObject(item->hdc, old_pen);
                            Win32::SelectObject(item->hdc, old_brush);
                            Win32::DeleteObject(pen);

                            return Win32::DoDefault;
                        }

                        if (push_checkedbrush == nullptr)
                            push_checkedbrush = CreateGradientBrush(Win32::GetRGB(0, 0, 180), Win32::GetRGB(0, 222, 200), item);

                        Win32::HPEN pen = Win32::CreatePen(Win32::PsInsideFrame, 0, Win32::GetRGB(0, 0, 0));
                        Win32::HGDIOBJ old_pen = Win32::SelectObject(item->hdc, pen);
                        Win32::HGDIOBJ old_brush = Win32::SelectObject(item->hdc, push_checkedbrush);

                        Win32::RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 10, 10);

                        Win32::SelectObject(item->hdc, old_pen);
                        Win32::SelectObject(item->hdc, old_brush);
                        Win32::DeleteObject(pen);

                        return Win32::DoDefault;
                    }
                    else
                    {
                        if (item->uItemState & Win32::CdisHot)
                        {
                            if (push_hotbrush2 == nullptr)
                                push_hotbrush2 = CreateGradientBrush(Win32::GetRGB(255, 230, 0), Win32::GetRGB(245, 0, 0), item);

                            Win32::HPEN pen = Win32::CreatePen(Win32::PsInsideFrame, 0, Win32::GetRGB(0, 0, 0));

                            Win32::HGDIOBJ old_pen = Win32::SelectObject(item->hdc, pen);
                            Win32::HGDIOBJ old_brush = Win32::SelectObject(item->hdc, push_hotbrush2);

                            Win32::RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 10, 10);

                            Win32::SelectObject(item->hdc, old_pen);
                            Win32::SelectObject(item->hdc, old_brush);
                            Win32::DeleteObject(pen);

                            return Win32::DoDefault;
                        }

                        if (push_uncheckedbrush == nullptr)
                            push_uncheckedbrush = CreateGradientBrush(Win32::GetRGB(255, 180, 0), Win32::GetRGB(180, 0, 0), item);

                        Win32::HPEN pen = Win32::CreatePen(Win32::PsInsideFrame, 0, Win32::GetRGB(0, 0, 0));

                        Win32::HGDIOBJ old_pen = Win32::SelectObject(item->hdc, pen);
                        Win32::HGDIOBJ old_brush = Win32::SelectObject(item->hdc, defaultbrush);

                        Win32::RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 10, 10);

                        Win32::SelectObject(item->hdc, old_pen);
                        Win32::SelectObject(item->hdc, old_brush);
                        Win32::DeleteObject(pen);

                        return Win32::DoDefault;
                    }
                }

                return Win32::DoDefault;
            }

            case Win32::Messages::Destroy:
            {
                if (defaultbrush)
                    Win32::DeleteObject(defaultbrush);
                if (selectbrush)
                    Win32::DeleteObject(selectbrush);
                if (hotbrush)
                    Win32::DeleteObject(hotbrush);
                if (push_checkedbrush)
                    Win32::DeleteObject(push_checkedbrush);
                if (push_hotbrush1)
                    Win32::DeleteObject(push_hotbrush1);
                if (push_hotbrush2)
                    Win32::DeleteObject(push_hotbrush2);
                if (push_uncheckedbrush)
                    Win32::DeleteObject(push_uncheckedbrush);
                Win32::PostQuitMessage(0);
                return 0;
            }

            case Win32::Messages::Close:
            {
                Win32::DestroyWindow(hwnd);
                break;
            }
                
            default:
                return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
        }
        return 0;
    }

    export int Run()
    {
        const wchar_t ClassName[] = L"Main_Window";
        WNDCLASSEXW wc{
            .cbSize = sizeof(WNDCLASSEX),
            .lpfnWndProc = 
                MainWindow,
                /*[](Win32::HWND hwnd, Win32::UINT msg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> long long 
                { 
                    switch (msg)
                    {
                        case Win32::Messages::Close:
                            Win32::PostQuitMessage(0);
                            return 0;

                        default:
                            return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
                    }
                    return 0;
                },*/
            .hInstance = Win32::GetModuleHandleW(nullptr),
            .hIcon = Win32::LoadIconW(nullptr, Win32::IdiApplication()),
            .hCursor = Win32::LoadCursorW(nullptr, Win32::Cursors::Arrow),
            .hbrBackground = Win32::GetSysColorBrush(Win32::ColorWindow + 1),
            .lpszClassName = ClassName,
            .hIconSm = Win32::LoadIconW(nullptr, Win32::IdiApplication())
        };

        if (!Win32::RegisterClassExW(&wc))
        {
            Win32::MessageBoxW(nullptr, L"Window Registration Failed!", L"Error", Win32::MessageBoxes::IconExclamation | Win32::MessageBoxes::OK);
            std::terminate();
        }

        Win32::HWND hwnd = Win32::CreateWindowExW(
            Win32::ExtendedStyles::ClientEdge,
            ClassName, 
            L"Window", 
            Win32::Styles::OverlappedWindow,
            Win32::Cw_UseDefault,
            Win32::Cw_UseDefault,
            368, 
            248, 
            nullptr, 
            nullptr, 
            Win32::GetModuleHandleW(nullptr),
            nullptr
        );

        if (hwnd == nullptr)
        {
            Win32::MessageBoxW(nullptr, L"Window Creation Failed!", L"Error!", Win32::MessageBoxes::IconExclamation | Win32::MessageBoxes::OK);
            std::terminate();
        }

        Win32::ShowWindow(hwnd, Win32::Sw_ShowDefault);
        Win32::UpdateWindow(hwnd);

       
        MSG msg;
        while (Win32::GetMessageW(&msg, nullptr, 0, 0) > 0)
        {
            Win32::TranslateMessage(&msg);
            Win32::DispatchMessageW(&msg);
        }

        return msg.message;
    }
}
