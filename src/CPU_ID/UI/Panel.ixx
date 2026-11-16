export module CpuId:UI.Panel;
import std;
import Win32;
import :UI.WindowClass;
import :UI.Common;

export namespace UI
{
    class Panel
    {
    public:
		~Panel()
		{
            Destroy();
		}

        Panel(Win32::HWND hwnd)
            : m_hwnd(hwnd)
        {}

		Panel(const Panel&) = delete;
		auto operator=(const Panel&) -> Panel & = delete;

        Panel(Panel&& other) noexcept
            : m_hwnd{ other.m_hwnd }
        {
            other.m_hwnd = nullptr;
            if (m_hwnd)
                Win32::SetWindowLongPtrW(m_hwnd, Win32::Gwlp::UserData, reinterpret_cast<Win32::DWORD_PTR>(this));
        }
        auto operator=(Panel&& other) noexcept -> Panel&
        {
            if (this == &other)
                return *this;
            Destroy();
            m_hwnd = std::exchange(other.m_hwnd, nullptr);
            if (m_hwnd)
                Win32::SetWindowLongPtrW(m_hwnd, Win32::Gwlp::UserData, reinterpret_cast<Win32::DWORD_PTR>(this));
            return *this;
        }

        void Destroy()
        {
            if (m_hwnd)
            {
                auto tmp = m_hwnd;
                release();
                Win32::DestroyWindow(tmp);
            }
        }

        auto HandleMessage(
            Win32::HWND hwnd,
            unsigned uMsg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam
        ) -> std::optional<Win32::LRESULT>
        {
            switch (uMsg)
            {
			    case Win32::Messages::Command:
			    {
					auto originalHwnd = reinterpret_cast<Win32::HWND>(lParam);
					return Win32::SendMessageW(
						originalHwnd,
						Win32::Messages::ReflectedCommand,
						wParam,
						lParam
					);
			    }

                case Win32::Messages::CtlColorStatic:
                {
                    return reinterpret_cast<Win32::LRESULT>(Win32::GetStockObject(Win32::Brushes::White));
                }
                default:
                    return std::nullopt;
            }
        }

        void release()
        {
            if (m_hwnd)
            {
                Win32::SetWindowLongPtrW(m_hwnd, Win32::Gwlp::UserData, 0);
                m_hwnd = nullptr;
            }
        }

        auto GetHwnd() const noexcept -> Win32::HWND
        {
            return m_hwnd;
        }

        auto SetFontOnChildren(Win32::HFONT font)
        {
            Win32::EnumChildWindows(
                GetHwnd(),
                [](Win32::HWND child, Win32::LPARAM fontValue) -> Win32::BOOL
                {
                    Win32::SendMessageW(
                        child,
                        Win32::Messages::SetFont,
                        static_cast<Win32::WPARAM>(fontValue),
                        true
                    );
                    return true;
                },
                reinterpret_cast<Win32::LPARAM>(font)
            );
        }

        auto GetClientDimensions() const -> Dimensions
        {
            if (not m_hwnd)
                return {};
            auto rect = Win32::RECT{};
            Win32::GetClientRect(GetHwnd(), &rect);
            return { static_cast<unsigned>(rect.right - rect.left), static_cast<unsigned>(rect.bottom - rect.top) };
        }

    protected:
        Win32::HWND m_hwnd = nullptr;
    };

	using PanelWindowClass = WindowClass<"PanelWindowClass", Panel, false>;
}