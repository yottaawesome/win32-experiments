export module CpuId:UI.TopLevelWindow;
import std;
import Win32;
import :UI.Common;

export namespace UI
{
    class TopLevelWindow
    {
    public:
        ~TopLevelWindow()
        {
            Destroy();
        }

        TopLevelWindow(Win32::HWND hwnd)
            : m_hwnd(hwnd)
        {}

		TopLevelWindow(const TopLevelWindow&) = delete;
        TopLevelWindow& operator=(const TopLevelWindow&) = delete;

		TopLevelWindow(TopLevelWindow&& other) noexcept
			: m_hwnd{ other.m_hwnd }
		{
			other.m_hwnd = nullptr;
            if (m_hwnd)
                Win32::SetWindowLongPtrW(m_hwnd, Win32::Gwlp::UserData, reinterpret_cast<Win32::DWORD_PTR>(this));
		}
		auto operator=(TopLevelWindow&& other) noexcept -> TopLevelWindow&
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

		[[nodiscard]]
        auto HandleMessage(
            Win32::HWND hwnd,
            unsigned uMsg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam
        ) -> std::optional<Win32::LRESULT>
        {
            switch (uMsg)
            {
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

		auto Show() const -> void
		{
            if(m_hwnd)
			    Win32::ShowWindow(m_hwnd, Win32::ShowWindowFlags::Show);
		}

		auto Hide() const -> void
		{
			if (m_hwnd)
				Win32::ShowWindow(m_hwnd, Win32::ShowWindowFlags::Hide);
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

    private:
        Win32::HWND m_hwnd = nullptr;
    };
}