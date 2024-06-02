export module samples;
import shared;

#pragma comment(lib, "Comctl32.lib")
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace BasicWindow
{
    constexpr std::wstring_view ClassName = L"MainWClass";
    constexpr std::wstring_view WindowName = L"Main Window";

    using namespace Win32;

    LRESULT __stdcall MainWndProc(HWND hwnd, UINT type, WPARAM wParam, LPARAM lParam)
    {
        switch (type)
        {
            case Messages::Close:
                DestroyWindow(hwnd);
                break;
            case Messages::Destroy:
                PostQuitMessage(0);
            default:
                return DefWindowProcW(hwnd, type, wParam, lParam);
        }
        return 0;
    }

    HWND CreateTabControl(HWND hwndParent, HINSTANCE g_hInst)
    {
        constexpr auto DAYS_IN_WEEK = 7;

        // Initialize common controls.
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = Icc_Tab_Classes;
        InitCommonControlsEx(&icex);

        // Get the dimensions of the parent window's client area, and 
        // create a tab control child window of that size. Note that g_hInst
        // is the global instance handle.
        RECT rcClient;
        GetClientRect(hwndParent, &rcClient);
        HWND hwndTab = Win32::CreateWindowExW(
            0,
            Wc_TabControl,
            L"",
            WindowStyles::Ws_Child | WindowStyles::Ws_ClipSiblings | WindowStyles::Ws_Visible,
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
        for (int i = 0; i < DAYS_IN_WEEK; i++)
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

    bool InitApplication(HINSTANCE hinstance, std::wstring_view className)
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

    HWND CreateMainWindow(HINSTANCE hInst, std::wstring_view className, std::wstring_view windowName)
    {
        HWND hwndMain = CreateWindowExW(
            0,                                              // no extended styles           
            className.data(),                               // class name                   
            windowName.data(),                              // window name                  
            WindowStyles::Ws_OverlappedWindow | WindowStyles::Ws_HScroll | WindowStyles::Ws_VScroll,  // overlapped window horizontal scroll and vertical scroll bar          
            Cw_UseDefault,                                  // default horizontal position  
            Cw_UseDefault,                                  // default vertical position    
            Cw_UseDefault,                                  // default width                
            Cw_UseDefault,                                  // default height               
            (HWND)nullptr,                                  // no parent or owner window    
            (HMENU)nullptr,                                 // class menu used              
            hInst,                                          // instance handle              
            nullptr                                         // no window creation data      
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

        if (not CreateTabControl(hwndMain, hinst))
            return 1;

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