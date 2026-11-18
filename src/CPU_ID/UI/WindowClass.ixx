export module CpuId:UI.WindowClass;
import std;
import Win32;
import :Error;
import :Strings;

export namespace UI
{
	struct NoOperation {} constexpr NoOp{};

    template<
        Strings::FixedString ClassName, 
        typename TWindow, 
        bool HandleDestroy, 
        Win32::DWORD ClassAdditionalStyles = 0,
		Win32::DWORD ClassAdditionalExStyles = 0
    >
    struct WindowClass
    {
    public:
        ~WindowClass()
        {
            Unregister();
        }

        WindowClass(const NoOperation&) noexcept {}

        WindowClass()
        {
            Register();
        }

		WindowClass(const WindowClass&) = delete;
        auto operator=(const WindowClass&) -> WindowClass& = delete;

        auto CreateWindow(
			this auto&& self,
            const std::wstring& title,
            Win32::DWORD styles,
            Win32::DWORD exStyles,
			unsigned x,
			unsigned y,
            unsigned width,
            unsigned height,
			Win32::HWND parent
        ) -> std::unique_ptr<TWindow>
        {
            auto hInstance = Win32::GetModuleHandleW(nullptr);
            if (not hInstance)
                throw Error::Win32Error{ Win32::GetLastError(), "GetModuleHandleW failed" };

            auto window = std::make_unique<TWindow>();
            auto hwnd = Win32::CreateWindowExW(
                exStyles | ClassAdditionalExStyles,
                ClassName.ToView().data(),
                title.c_str(),
                styles | ClassAdditionalStyles,
                x, y,
                width, height,
                parent,
                nullptr,
                hInstance,
                window.get());
            if (not hwnd)
            {
                window->Detach();
                throw Error::Win32Error{ Win32::GetLastError(), "CreateWindowExW failed" };
            }
            return window;
        }

    private:
        auto Unregister(this auto&& self) -> void
        {
			if (not self.m_atom)
				return;
            if (not Win32::UnregisterClassW(ClassName.ToView().data(), Win32::GetModuleHandleW(nullptr)))
                throw Error::Win32Error{ Win32::GetLastError(), "UnregisterClassW failed" };
			self.m_atom = 0;
        }

        auto Register(this auto&& self) -> void
        {
            auto hInstance = Win32::GetModuleHandleW(nullptr);
            if (not hInstance)
                throw Error::Win32Error{ Win32::GetLastError(), "GetModuleHandleW failed" };
            auto wc = Win32::WNDCLASSEXW{
                .cbSize = sizeof(Win32::WNDCLASSEXW),
                .style = 0,
                .lpfnWndProc = WndProc,
                .cbClsExtra = 0,
                .cbWndExtra = 0,
                .hInstance = hInstance,
                .hIcon = nullptr,
                .hCursor = nullptr,
                .hbrBackground = static_cast<Win32::HBRUSH>(Win32::GetStockObject(Win32::Brushes::White)),
                .lpszMenuName = nullptr,
                .lpszClassName = ClassName.ToView().data(),
                .hIconSm = nullptr
            };
			self.m_atom = Win32::RegisterClassExW(&wc);
            if (not self.m_atom)
                throw Error::Win32Error{ Win32::GetLastError(), "RegisterClassExW failed" };
        }

    protected:
		Win32::ATOM m_atom = 0;

        static auto WndProc(
            Win32::HWND hwnd, 
            unsigned uMsg, 
            Win32::WPARAM wParam, 
            Win32::LPARAM lParam
        ) noexcept -> Win32::LRESULT
        try
        {
            auto pThis = static_cast<TWindow*>(nullptr);
            if (uMsg == Win32::Messages::NonClientCreate)
            {
                auto* pCreate = reinterpret_cast<Win32::CREATESTRUCT*>(lParam);
                pThis = static_cast<TWindow*>(pCreate->lpCreateParams);
                if (not pThis)
                    return 0;
                pThis->Attach(hwnd);
            }
            else
            {
                pThis = reinterpret_cast<TWindow*>(Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp::UserData));
                if (pThis and uMsg == Win32::Messages::NonClientDestroy)
                    pThis->Detach();
            }

            if (pThis)
            {
                auto result = pThis->HandleMessage(hwnd, uMsg, wParam, lParam);
                if (result and uMsg != Win32::Messages::NonClientDestroy)
                {
                    return *result;
                }
            }

			if constexpr (HandleDestroy)
				if (uMsg == Win32::Messages::Destroy)
					return (Win32::PostQuitMessage(0), 0);
            return Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }
        catch (const std::exception& error)
        {
            Error::ErrorExit(1, error.what());
        }
        catch (...)
        {
            Error::ErrorExit(1, "Unknown exception in window procedure");
        }
    };
}