export module samples;
import shared;

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

    HWND CreateWindow(HINSTANCE hInst, std::wstring_view className, std::wstring_view windowName)
    {
        HWND hwndMain = CreateWindowExW(
            0,                                              // no extended styles           
            className.data(),                               // class name                   
            windowName.data(),                              // window name                  
            Ws_OverlappedWindow | Ws_HScroll | Ws_VScroll,  // overlapped window horizontal scroll and vertical scroll bar          
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

        HWND hwndMain = CreateWindow(hinst, ClassName, WindowName);
        if (not hwndMain)
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