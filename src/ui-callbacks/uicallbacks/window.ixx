export module uicallbacks:window;
import std;
import :win32;
import :error;

export namespace UiCallbacks::UI
{
    class Window
    {
    public:
        // In Window class:
        using MessageHandler = std::move_only_function<Win32::LRESULT(Win32::HWND, Win32::WPARAM, Win32::LPARAM)>;
        using CommandHandler = std::move_only_function<Win32::LRESULT(Win32::WORD notifyCode)>;

        ~Window()
        {
            Destroy();
        }

        // Not copyable
        Window(const Window&) = delete;
        auto operator=(const Window&) -> Window & = delete;

        // Movable
        Window(Window&& other)
        {
            MoveFrom(other);
        }
        auto operator=(Window&& other) -> Window&
        {
            MoveFrom(other);
            return *this;
        }

        auto MoveFrom(this auto&& self, Window& other) -> void
        {
            if (&self == &other)
                return;
            self.Destroy();
            self.window = other.Detach();
            if (self.window)
                Win32::SetWindowLongPtrW(self.window, Win32::Gwlp::UserData, reinterpret_cast<Win32::LONG_PTR>(&self));
        }

        Window() = default;

        Window(std::wstring_view title, std::uint32_t width, std::uint32_t height, Win32::DWORD extendedStyles = 0, Win32::DWORD additionalStyles = 0)
        {
            Init(title, width, height, extendedStyles, additionalStyles);
        }

        auto Detach(this auto&& self) -> Win32::HWND
        {
            if (not self.window)
                return nullptr;
            auto hwnd = self.window;
            Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp::UserData, 0);
            self.window = nullptr;
            return hwnd;
        }

        auto On(this auto&& self, Win32::UINT message, MessageHandler handler) -> void
        {
            self.handlers[message] = std::move(handler);
        }

        auto RemoveHandler(Win32::UINT message) -> void
        {
            handlers.erase(message);
		}

        auto RemoveCommand(WORD id) -> void
        {
            commandHandlers.erase(id);
        }

        auto Destroy(this auto&& self) -> void
        {
            if (self.window)
                Win32::DestroyWindow(self.window);
            self.window = nullptr;
        }

        auto HandleMessage(this auto&& self, Win32::UINT type, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
        {
            if (auto it = self.handlers.find(type); it != self.handlers.end())
                return it->second(self.window, wParam, lParam);

            if (type == Win32::WindowMessages::Command)
            {
                auto controlId = Win32::GetLowWord(wParam);
                auto notifyCode = Win32::GetHighWord(wParam);
                if (auto it = self.commandHandlers.find(controlId); it != self.commandHandlers.end())
                    return it->second(notifyCode);
            }

            switch (type)
            {
            case Win32::WindowMessages::Close:
                return self.OnClose();
            case Win32::WindowMessages::Destroy:
                return self.OnDestroy();
            }
            return Win32::DefWindowProcW(self.window, type, wParam, lParam);
        }

        auto OnCommand(this auto&& self, Win32::WORD controlId, CommandHandler handler) -> void
        {
            self.commandHandlers[controlId] = std::move(handler);
        }

        auto OnClose(this auto&& self) -> Win32::LRESULT
        {
            Win32::DestroyWindow(self.window);
            return 0;
        }

        auto OnDestroy(this auto&& self) -> Win32::LRESULT
        {
            Win32::PostQuitMessage(0);
            return 0;
        }

        constexpr auto ClassName(this auto&&) -> std::wstring_view
        {
            return L"UICallbacks";
		}

        auto Init(
            this auto&& self, 
            std::wstring_view title, 
            std::uint32_t width, 
            std::uint32_t height, 
            Win32::DWORD extendedStyles = 0, 
            Win32::DWORD additionalStyles = 0
        ) -> void
        {
            if (self.window)
                throw UiCallbacks::Error::Win32Error{ Win32::ErrorCodes::InvalidParameter, "Window is already initialized" };
            self.RegisterWindowClass();
            self.window = Win32::CreateWindowExW(
                extendedStyles,
                self.ClassName().data(),
                title.data(),
                Win32::WindowStyles::OverlappedWindow | additionalStyles,
                Win32::CwConstants::UseDefault,
                Win32::CwConstants::UseDefault,
                width,
                height,
                nullptr,
                nullptr,
                Win32::GetModuleHandleW(nullptr),
                &self
            );
            if (not self.window)
                throw UiCallbacks::Error::Win32Error{ Win32::GetLastError(), "Failed to create window" };

            Win32::ShowWindow(self.window, Win32::ShowWindowOptions::Default);
            Win32::UpdateWindow(self.window);
        }

        auto DoMessageLoop(this auto&& self) -> Win32::LRESULT
        {
            auto msg = Win32::MSG{};
			while (Win32::GetMessageW(&msg, nullptr, 0, 0) > 0)
            {
                Win32::TranslateMessage(&msg);
                Win32::DispatchMessageW(&msg);
            }
			return msg.wParam;
        }

        // onIdle returns true if there's more idle work to do, false to block until next message.
        auto DoMessageLoop(this auto&& self, std::invocable auto&& onIdle) -> Win32::LRESULT
            requires std::convertible_to<std::invoke_result_t<decltype(onIdle)>, bool>
        {
            auto msg = Win32::MSG{};
            auto running = true;

            /*
			* Alternative, which is more responsive
            * while (true)
            {
                if (Win32::PeekMessageW(&msg, nullptr, 0, 0, Win32::PeekMessageOptions::Remove))
                {
                    if (msg.message == Win32::WindowMessages::Quit)
                        break;
                    Win32::TranslateMessage(&msg);
                    Win32::DispatchMessageW(&msg);
                }
                else if (not onIdle())
                {
                    Win32::WaitMessage();
                }
            }
            */
            while (running)
            {
                while (Win32::PeekMessageW(&msg, nullptr, 0, 0, Win32::PeekMessageOptions::Remove))
                {
                    if (msg.message == Win32::WindowMessages::Quit)
                    {
                        running = false;
                        break;
                    }
                    Win32::TranslateMessage(&msg);
                    Win32::DispatchMessageW(&msg);
                }
                if (running and not onIdle())
                {
                    Win32::WaitMessage();
                }
            }
			return msg.wParam;
		}

        auto Handle(this auto&& self) -> Win32::HWND
        {
            return self.window;
		}

    protected:
        auto RegisterWindowClass(this auto&& self) -> void
        {
            using T = std::remove_cvref_t<decltype(self)>;

            auto wndclass = Win32::WNDCLASSEXW{
                .cbSize = sizeof(Win32::WNDCLASSEXW),
                .lpfnWndProc =
                    [](Win32::HWND hwnd, Win32::UINT type, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
                    {
                        auto object = static_cast<T*>(nullptr);

                        if (type == Win32::WindowMessages::NonClientCreate)
                        {
                            auto pCreate = reinterpret_cast<Win32::CREATESTRUCT*>(lParam);
                            object = reinterpret_cast<T*>(pCreate->lpCreateParams);
                            Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp::UserData, reinterpret_cast<Win32::LONG_PTR>(object));
                            object->window = hwnd;
                        }
                        else
                        {
                            object = reinterpret_cast<T*>(Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp::UserData));
                        }

                        if (object)
                        {
                            auto result = object->HandleMessage(type, wParam, lParam);
                            if (type == Win32::WindowMessages::NonClientDestroy)
                            {
                                Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp::UserData, 0);
                                object->window = nullptr;
                            }
                            return result;
                        }
                        return Win32::DefWindowProcW(hwnd, type, wParam, lParam);
                    },
                .hInstance = Win32::GetModuleHandleW(nullptr),
                .lpszClassName = self.ClassName().data(),
            };
            if (not Win32::RegisterClassExW(&wndclass))
                if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::ClassAlreadyExists)
                    throw UiCallbacks::Error::Win32Error{ lastError, "Failed to register window class" };
        }

        Win32::HWND window = nullptr;
        std::unordered_map<Win32::UINT, MessageHandler> handlers;
        std::unordered_map<Win32::UINT, CommandHandler> commandHandlers;
    };
}