export module app:toplevelwindow;
import std;
import :win32;
import :common;

export namespace UI
{
	struct TopLevelWindow : Window, Textable
	{
		//
		// Creates the window.
		auto Create(this auto&& self) -> decltype(auto)
		{
			static auto done =
				[&self] -> bool
				{
					auto wndClass = self.GetClass();
					wndClass.lpfnWndProc = WindowProc<std::remove_cvref_t<decltype(self)>>;
					return Win32::RegisterClassExW(&wndClass) ? true : throw Error::Win32Error{};
				}();

			auto args = self.GetCreationArgs();
			auto hwnd = Win32::CreateWindowExW(
				args.ExtendedStyle,
				self.ClassName.data(),
				args.WindowName,
				args.Style,
				args.X,
				args.Y,
				args.Width,
				args.Height,
				args.ParentWindow,
				args.Menu,
				Win32::GetModuleHandleW(nullptr),
				&self
			);
			self.m_window = HwndUniquePtr(hwnd);
			if constexpr (requires { self.Init(); })
				self.Init();
			return std::forward<decltype(self)>(self);
		}

		//
		// Applications call this to retrieve messages.
		auto MainLoop(this auto&& self) -> Win32::LRESULT
		{
			auto msg = Win32::MSG{};
			while (Win32::GetMessageW(&msg, nullptr, 0, 0))
			{
				Win32::TranslateMessage(&msg);
				Win32::DispatchMessageW(&msg);
			}
			return msg.wParam;
		}

	protected:
		//
		// The main Window proc.
		template<typename TWindow>
		static auto WindowProc(Win32::HWND hwnd, unsigned uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
		{
			if (uMsg == Win32::Messages::NonClientCreate)
			{
				auto pCreate = reinterpret_cast<Win32::CREATESTRUCT*>(lParam);
				auto pThis = static_cast<TWindow*>(pCreate->lpCreateParams);
				Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData, (Win32::LONG_PTR)pThis);
				return Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
			}
			else
			{
				auto pThis = reinterpret_cast<TWindow*>(GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData));
				return pThis
					? pThis->HandleMessage(hwnd, uMsg, wParam, lParam)
					: Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
			}
		}

		static constexpr auto KnownMessages = std::array{
			Win32::Messages::Destroy,
			Win32::Messages::Paint,
			Win32::Messages::KeyUp,
			Win32::Messages::Command
		};

		//
		// Called by WindowProc, which then dispatches the message to either the generic handler
		// or specific handlers by subclasses.
		auto HandleMessage(
			this auto&& self,
			Win32::HWND hwnd,
			unsigned msgType,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam
		) -> Win32::LRESULT
		{
			return[&self, hwnd, msgType, wParam, lParam]<size_t...Is>(std::index_sequence<Is...>)
			{
				auto result = Win32::LRESULT{};
				auto handled = (... or 
					[=, &self, &result]<typename TMsg = Win32Message<std::get<Is>(KnownMessages)>> -> bool
					{
						if (TMsg::uMsg != msgType)
						{
							return false;
						}
						else if constexpr (Handles<decltype(self), TMsg>)
						{
							return (result = self.OnMessage(TMsg{ hwnd, wParam, lParam }), true);
						}
						else
						{
							return false;
						}
					}());
				if (handled)
					return result;
				return msgType == Win32::Messages::Destroy
					? (Win32::PostQuitMessage(0), 0)
					: Win32::DefWindowProcW(hwnd, msgType, wParam, lParam);
			}(std::make_index_sequence<KnownMessages.size()>());
		}
	};
}
