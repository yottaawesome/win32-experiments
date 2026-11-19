export module CpuId:UI.Control;
import std;
import Win32;
import :Error;
import :UI.Functions;
import :UI.Common;

export namespace UI
{
	struct ControlArgs
	{
		unsigned ControlId = NextControlId();
		std::wstring Text;
		Win32::HWND Parent = nullptr;
		unsigned X = 0;
		unsigned Y = 0;
		unsigned Width = 0;
		unsigned Height = 0;
		Win32::DWORD WindowStyles = Win32::WindowStyles::Visible;
	};

    template<typename TImplementation>
    class ControlWindow
    {
    public:
		~ControlWindow()
		{
			Destroy();
		}

		ControlWindow() = default;

		ControlWindow(const ControlWindow&) = delete;
		auto operator=(const ControlWindow&) -> ControlWindow & = delete;

        void Destroy(this auto&& self)
        {
            if (self.m_hwnd)
                Win32::DestroyWindow(self.Detach());
        }

		auto Detach(this auto&& self) -> Win32::HWND
		{
			if (self.m_hwnd)
				Win32::RemoveWindowSubclass(self.m_hwnd, &ControlWindow::SubclassProc, 0);
			return std::exchange(self.m_hwnd, nullptr);
		}

        [[nodiscard]]
        auto GetHwnd(this auto&& self) noexcept -> Win32::HWND
        {
            return self.m_hwnd;
        }

        [[nodiscard]]
        auto GetClientRect(this auto&& self) -> Win32::RECT
        {
            if (not self.m_hwnd)
                return {};
            auto rect = Win32::RECT{};
            if (not Win32::GetClientRect(self.GetHwnd(), &rect))
                throw Error::Win32Error{ Win32::GetLastError(), "GetClientRect failed" };
            return rect;
        }

        [[nodiscard]]
        auto GetClientDimensions(this auto&& self) -> Dimensions
        {
            auto rect = self.GetClientRect();
            return { static_cast<unsigned>(rect.right - rect.left), static_cast<unsigned>(rect.bottom - rect.top) };
        }

    protected:
        [[nodiscard]]
        auto HandleMessage(
			this auto&& self,
            Win32::HWND hwnd,
            unsigned uMsg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam
        ) -> std::optional<Win32::LRESULT>
        {
            return std::nullopt;
        }

        void Create(this auto&& self, std::wstring_view className, ControlArgs args)
        {
            self.m_hwnd = Win32::CreateWindowExW(
                0,
                className.data(),
                args.Text.c_str(),
                Win32::WindowStyles::Child | args.WindowStyles,
                args.X, args.Y,
                args.Width, args.Height,
                args.Parent,
                reinterpret_cast<Win32::HMENU>(static_cast<Win32::ULONG_PTR>(args.ControlId)),
                Win32::GetModuleHandleW(nullptr),
                nullptr);
            if (not self.m_hwnd)
                throw Error::Win32Error{ Win32::GetLastError(), "CreateWindowExW failed" };
            if (not Win32::SetWindowSubclass(self.m_hwnd, &SubclassProc, 0, reinterpret_cast<Win32::DWORD_PTR>(&self)))
                throw std::runtime_error{ "Failed to track control window destruction" };
        }

        //
		// Core subclass procedure, passes messages to the derived class for handling.
		// Detaches the instance from the window when the window is destroyed.
        static auto SubclassProc(
            Win32::HWND hwnd,
            unsigned uMsg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam,
            Win32::UINT_PTR,
            Win32::DWORD_PTR dwRefData
        ) noexcept -> Win32::LRESULT
        try
        {
            if (auto pThis = reinterpret_cast<TImplementation*>(dwRefData); not pThis)
            {
                return Win32::DefSubclassProc(hwnd, uMsg, wParam, lParam);
            }
            else if (uMsg == Win32::Messages::NonClientDestroy)
            {
                pThis->Detach();
                return Win32::DefSubclassProc(hwnd, uMsg, wParam, lParam);
            }
            else
            {
                auto result = pThis->HandleMessage(hwnd, uMsg, wParam, lParam);
				return result ? *result : Win32::DefSubclassProc(hwnd, uMsg, wParam, lParam);
            }
        }
        catch (const std::exception& error)
        {
            Error::ErrorExit(1, error.what());
        }
        catch (...)
        {
            Error::ErrorExit(1, "Unknown exception in control subclass procedure");
        }

    //
	// Protected members   
    protected:
        Win32::HWND m_hwnd = nullptr;
    };
}
