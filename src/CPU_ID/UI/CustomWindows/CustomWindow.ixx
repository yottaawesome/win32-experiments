export module CpuId:UI.CustomWindow;
import std;
import Win32;
import :Error;
import :UI.Common;

export namespace UI
{
	class CustomWindow
	{
		// Special member functions
	public:
        ~CustomWindow()
        {
            Destroy();
        }

		CustomWindow() = default;

        CustomWindow(const CustomWindow&) = delete;
        auto operator=(const CustomWindow&) -> CustomWindow & = delete;

        CustomWindow(CustomWindow&&) = delete;
        auto operator=(CustomWindow&&) -> CustomWindow& = delete;

        //
		// Public methods
    public:
        auto Attach(this auto&& self, Win32::HWND hwnd) -> void
        {
            if (not hwnd)
                throw std::invalid_argument{ "window handle is null" };
            if (self.m_hwnd)
                throw std::logic_error{ "window handle is already attached" };
            Win32::SetLastError(0);
            if (not Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp::UserData, reinterpret_cast<Win32::LONG_PTR>(&self)))
                if (auto error = Win32::GetLastError(); error)
                    throw Error::Win32Error{ error, "SetWindowLongPtrW failed" };
            self.m_hwnd = hwnd;
        }

        auto Detach() noexcept -> void
        {
            m_hwnd = nullptr;
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

        auto Show(this auto&& self) -> decltype(self)
        {
            if (self.m_hwnd)
                Win32::ShowWindow(self.m_hwnd, Win32::ShowWindowFlags::Show);
			return self;
        }

        auto Hide(this auto&& self) -> decltype(self)
        {
            if (self.m_hwnd)
                Win32::ShowWindow(self.m_hwnd, Win32::ShowWindowFlags::Hide);
			return self;
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

		auto GetClientRect() const -> Win32::RECT
		{
			if (not m_hwnd)
				return {};
			auto rect = Win32::RECT{};
			if (not Win32::GetClientRect(GetHwnd(), &rect))
                throw Error::Win32Error{Win32::GetLastError(), "GetClientRect failed"};
			return rect;
		}

        auto GetClientDimensions() const -> Dimensions
        {
            if (not m_hwnd)
                return {};
            auto rect = Win32::RECT{};
            Win32::GetClientRect(GetHwnd(), &rect);
            return { static_cast<unsigned>(rect.right - rect.left), static_cast<unsigned>(rect.bottom - rect.top) };
        }

        //
		// Protected members
	protected:
		Win32::HWND m_hwnd = nullptr;
	};
}
