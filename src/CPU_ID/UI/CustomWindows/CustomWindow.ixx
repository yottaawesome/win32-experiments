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
            self.m_hwnd = hwnd;
        }

        auto Detach(this auto&& self) noexcept -> Win32::HWND
        {
			return std::exchange(self.m_hwnd, nullptr);
        }

        void Destroy(this auto&& self)
        {
            if (self.m_hwnd)
                Win32::DestroyWindow(self.Detach());
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

        auto GetHwnd(this auto&& self) noexcept -> Win32::HWND
        {
            return self.m_hwnd;
        }

		auto GetClientRect(this auto&& self) -> Win32::RECT
		{
			if (not self.m_hwnd)
				return {};
			auto rect = Win32::RECT{};
			if (not Win32::GetClientRect(self.GetHwnd(), &rect))
                throw Error::Win32Error{Win32::GetLastError(), "GetClientRect failed"};
			return rect;
		}

        auto GetClientDimensions(this auto&& self) -> Dimensions
        {
            auto rect = self.GetClientRect();
            return { static_cast<unsigned>(rect.right - rect.left), static_cast<unsigned>(rect.bottom - rect.top) };
        }

        //
		// Protected members
	protected:
		Win32::HWND m_hwnd = nullptr;
	};
}
