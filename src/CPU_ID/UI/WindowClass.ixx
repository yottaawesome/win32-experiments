export module CpuId:UI.WindowClass;
import std;
import Win32;
import :Error;
import :Strings;

export namespace UI
{
    template<typename T>
	struct AlwaysFalse : std::false_type {};

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
			constexpr auto GetUserData = 
                []<typename TArg>(TArg arg) static noexcept -> TWindow*
				{
                    if (not arg)
                        return nullptr;

                    if constexpr (std::same_as<TArg, Win32::HWND>)
                        return reinterpret_cast<TWindow*>(Win32::GetWindowLongPtrW(arg, Win32::Gwlp::UserData));
					else if constexpr (std::same_as<TArg, Win32::CREATESTRUCT*>)
                        return static_cast<TWindow*>(arg->lpCreateParams);
                    else
						static_assert(AlwaysFalse<TArg>::value, "GetUserData: Unsupported argument type");
				};

            switch (uMsg)
            {
                case Win32::Messages::NonClientCreate:
                {
                    if (auto wndImpl = GetUserData(reinterpret_cast<Win32::CREATESTRUCT*>(lParam)); wndImpl)
                    {
                        Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp::UserData, reinterpret_cast<Win32::LONG_PTR>(wndImpl));
                        wndImpl->Attach(hwnd);
                    }
                    return Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
                }

                // Destroy is sent to the window before NonClientDestroy
                case Win32::Messages::Destroy:
                {
                    if constexpr (HandleDestroy)
                    {
                        Win32::PostQuitMessage(0);
                        return 0;
                    }
                    else if (auto wndImpl = GetUserData(hwnd); wndImpl)
                    {
                        auto result = wndImpl->HandleMessage(hwnd, uMsg, wParam, lParam);
                        return result ? *result : Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
                    }
                    else
                    {
                        return Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
                    }
                }

				case Win32::Messages::NonClientDestroy:
				{
					if (auto wndImpl = GetUserData(hwnd); wndImpl)
						wndImpl->Detach();
					Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp::UserData, 0);
					return Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
				}

                default:
                {
                    if (auto wndImpl = GetUserData(hwnd); wndImpl)
                    {
                        auto result = wndImpl->HandleMessage(hwnd, uMsg, wParam, lParam);
                        return result ? *result : Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
                    }
					else
					{
						return Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
					}
                }
            }
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