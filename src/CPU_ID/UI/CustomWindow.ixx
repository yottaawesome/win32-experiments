export module CpuId:UI.CustomWindow;
import std;
import Win32;
import :UI.Common;

export namespace UI
{
	class CustomWindow
	{
	public:
        ~CustomWindow()
        {
            Destroy();
        }

		CustomWindow() = default;

        CustomWindow(const CustomWindow&) = delete;
        auto operator=(const CustomWindow&) -> CustomWindow & = delete;

		CustomWindow(CustomWindow&& other) noexcept = default;
		auto operator=(CustomWindow&& other) noexcept -> CustomWindow & = default;

        void Destroy()
        {
            if (m_hwnd)
            {
                auto tmp = m_hwnd;
                release();
                Win32::DestroyWindow(tmp);
            }
        }

        auto Show() const -> void
        {
            if (m_hwnd)
                Win32::ShowWindow(m_hwnd, Win32::ShowWindowFlags::Show);
        }

        auto Hide() const -> void
        {
            if (m_hwnd)
                Win32::ShowWindow(m_hwnd, Win32::ShowWindowFlags::Hide);
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

        auto GetClientDimensions() const -> Dimensions
        {
            if (not m_hwnd)
                return {};
            auto rect = Win32::RECT{};
            Win32::GetClientRect(GetHwnd(), &rect);
            return { static_cast<unsigned>(rect.right - rect.left), static_cast<unsigned>(rect.bottom - rect.top) };
        }

	protected:
        auto Move(this auto&& self, CustomWindow& other) -> decltype(self)
        {
            if (&self == &other)
                return self;
            self.Destroy();
            self.m_hwnd = std::exchange(other.m_hwnd, nullptr);
            if (self.m_hwnd)
                Win32::SetWindowLongPtrW(self.m_hwnd, Win32::Gwlp::UserData, reinterpret_cast<Win32::DWORD_PTR>(&self));
            return self;
        }

		auto LinkHwndToThis() -> void
		{
			if (not m_hwnd)
				throw std::invalid_argument{ "window handle is null" };
			Win32::SetWindowLongPtrW(m_hwnd, Win32::Gwlp::UserData, reinterpret_cast<Win32::DWORD_PTR>(this));
		}

		Win32::HWND m_hwnd = nullptr;
	};
}
