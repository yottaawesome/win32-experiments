export module Samples:ObjectOrientedControl;
import std;
import Shared;
import Util;

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
            m_control.Handle = Win32::CreateWindowExW(
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
                Win32::GetModuleHandleW(nullptr),
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

        static auto __stdcall SubclassProc(
            Win32::HWND hwnd,
            Win32::UINT msg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam,
            Win32::UINT_PTR idSubclass,
            Win32::DWORD_PTR refData
        ) -> Win32::LRESULT
        {
            auto pThis = reinterpret_cast<Control<TType>*>(refData);
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
        static constexpr auto Class = std::wstring_view{ VClassName.View() };
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
            auto rc = Win32::RECT{};
            Win32::GetClientRect(Handle, &rc);
            Win32::MapWindowPoints(Handle, Win32::GetParent(Handle), (Win32::LPPOINT)&rc, 2);

            rc.left += deltaLeft;
            rc.top += deltaTop;

            Win32::SetWindowPos(Handle, nullptr, rc.left, rc.top, 0, 0, Win32::NoSize);
        }

        virtual auto Process(
            Win32::UINT msg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam,
            Win32::UINT_PTR uIdSubclass,
            Win32::DWORD_PTR dwRefData
        ) -> Win32::LRESULT
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

        auto Process(
            Win32::UINT msg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam,
            Win32::UINT_PTR uIdSubclass,
            Win32::DWORD_PTR dwRefData
        ) -> Win32::LRESULT override
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

        auto Init() -> Win32::HWND
        {
            auto wc = Win32::WNDCLASSEX{
                .cbSize = sizeof(Win32::WNDCLASSEX),
                .lpfnWndProc = WindowProc,
                .hInstance = Win32::GetModuleHandleW(nullptr),
                .hbrBackground = (Win32::HBRUSH)Win32::GetStockObject(Win32::White_Brush),   // white background brush
                .lpszClassName = TType::ClassName.data()
            };
            if (not Win32::RegisterClassExW(&wc))
                throw Error::Win32Error(Win32::GetLastError(), "Failed registering class");

            auto hwnd = Win32::CreateWindowExW(
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

        static auto __stdcall WindowProc(Win32::HWND hwnd, Win32::UINT uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
        {
            auto pThis = static_cast<Window<TType>*>(nullptr);

            if (uMsg == Win32::Messages::NonClientCreate)
            {
                auto pCreate = reinterpret_cast<Win32::CREATESTRUCT*>(lParam);
                pThis = (Window<TType>*)pCreate->lpCreateParams;
                Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData, (Win32::LONG_PTR)pThis);

                pThis->m_window.WindowHandle = hwnd;
            }
            else
            {
                pThis = (Window<TType>*)Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData);
            }

            return pThis
                ? pThis->m_window.HandleMessage(uMsg, wParam, lParam)
                : Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }
    };

    struct MainWindowType
    {
        MainWindowType(Win32::HWND handle) : WindowHandle(handle) {}

        static constexpr auto ClassName = std::wstring_view{ L"MainWindow" };
        static constexpr auto WindowName = std::wstring_view{ L"MainWindow" };

        Win32::HWND WindowHandle = nullptr;
        ButtonA m_buttonA{ WindowHandle };
        ButtonB m_buttonB{ WindowHandle };

        auto HandleMessage(Win32::UINT uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
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

    auto Run() -> int try
    {
        auto m = MainWindow{};
        return MessageLoop();
    }
    catch (const std::exception& ex)
    {
        std::println("{}", ex.what());
        return 1;
    }
}
