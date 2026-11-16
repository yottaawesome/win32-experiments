export module CpuId:UI.Control;
import std;
import Win32;
import :Error;
import :UI.Functions;

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
		Win32::DWORD WindowStyles = 0;
	};

    template<typename TImplementation>
    class Control
    {
    public:
		~Control()
		{
			Destroy();
		}

		Control() = default;

		Control(const Control&) = delete;
		auto operator=(const Control&) -> Control & = delete;

        void Destroy()
        {
            if (not m_hwnd)
                return;
            auto tmp = m_hwnd;
            m_hwnd = nullptr;
            Win32::RemoveWindowSubclass(tmp, &Control::SubclassProc, 0);
            Win32::DestroyWindow(tmp);
        }

		auto release() -> void
		{
			if (m_hwnd)
			{
				Win32::RemoveWindowSubclass(m_hwnd, &Control::SubclassProc, 0);
				m_hwnd = nullptr;
			}
		}

        [[nodiscard]]
        auto GetHwnd() const noexcept -> Win32::HWND
        {
            return m_hwnd;
        }

    protected:
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
            if (auto pThis = reinterpret_cast<TImplementation*>(dwRefData); pThis)
                if (auto result = pThis->HandleMessage(hwnd, uMsg, wParam, lParam); result)
                    return *result;
            return Win32::DefSubclassProc(hwnd, uMsg, wParam, lParam);
        }
        catch (const std::exception& error)
        {
            Error::ErrorExit(1, error.what());
        }
        catch (...)
        {
            Error::ErrorExit(1, "Unknown exception in control subclass procedure");
        }

        void Create(std::wstring_view className, ControlArgs args)
        {
            m_hwnd = Win32::CreateWindowExW(
                0,
                className.data(),
                args.Text.c_str(),
                Win32::WindowStyles::Child | Win32::WindowStyles::Visible | args.WindowStyles,
                args.X, args.Y,
                args.Width, args.Height,
                args.Parent,
                reinterpret_cast<Win32::HMENU>(static_cast<Win32::ULONG_PTR>(args.ControlId)),
                Win32::GetModuleHandleW(nullptr),
                nullptr);
            if (not m_hwnd)
                throw Error::Win32Error{ Win32::GetLastError(), "CreateWindowExW failed" };
            if (not Win32::SetWindowSubclass(m_hwnd, &SubclassProc, 0, reinterpret_cast<Win32::DWORD_PTR>(this)))
                throw std::runtime_error{ "Failed to track control window destruction" };
        }

        Win32::HWND m_hwnd = nullptr;
    };
}
