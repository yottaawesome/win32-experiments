export module Samples:ObjectOrientedWindowTemplate;
import std;
import Shared;

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
            auto wc = Win32::WNDCLASSEX{
                .cbSize = sizeof(Win32::WNDCLASSEX),
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

        virtual auto Window() const noexcept -> Win32::HWND final { return m_hwnd; }

    protected:
        virtual auto HandleMessage(Win32::UINT uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT = 0;

        static auto __stdcall WindowProc(Win32::HWND hwnd, Win32::UINT uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
        {
            auto pThis = static_cast<TDerived*>(nullptr);

            if (uMsg == Win32::Messages::NonClientCreate)
            {
                auto pCreate = reinterpret_cast<Win32::CREATESTRUCT*>(lParam);
                pThis = (TDerived*)pCreate->lpCreateParams;
                Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData, (Win32::LONG_PTR)pThis);

                pThis->m_hwnd = hwnd;
            }
            else
            {
                pThis = (TDerived*)Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData);
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
        static constexpr auto ClassName = std::wstring_view{ L"Sample Window Class" };

        auto HandleMessage(Win32::UINT uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT override
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
        window.Create(L"Blah", Win32::Styles::OverlappedWindow);
        return MessageLoop();
    }
}
