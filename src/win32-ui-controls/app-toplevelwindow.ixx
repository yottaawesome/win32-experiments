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
			static bool done =
				[&self]
				{
					Win32::WNDCLASSEXW wndClass = self.GetClass();
					wndClass.lpfnWndProc = WindowProc<std::remove_cvref_t<decltype(self)>>;
					return Win32::RegisterClassExW(&wndClass) ? true : throw Error::Win32Error{};
				}();

			CreateWindowArgs args = self.GetCreationArgs();
			Win32::HWND hwnd = Win32::CreateWindowExW(
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
					& self
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
			if constexpr (true) // Basic loop
			{
				Win32::MSG msg{};
				while (Win32::GetMessageW(&msg, nullptr, 0, 0))
				{
					Win32::TranslateMessage(&msg);
					Win32::DispatchMessageW(&msg);
				}
				return msg.wParam;
			}
			else // Optional processing loop
			{
				Win32::MSG msg{};
				while (msg.message != Win32::Messages::Quit)
				{
					if (Win32::PeekMessageW(&msg, self.m_hwnd, 0, 0, Win32::PeekMessageOptions::Remove))
					{
						Win32::TranslateMessage(&msg);
						Win32::DispatchMessageW(&msg);
					}
					else
					{
						// idle, add processing here
					}
				}
			}
		}

	protected:
		//
		// The main Window proc.
		template<typename TWindow>
		static auto WindowProc(Win32::HWND hwnd, unsigned uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
		{
			TWindow* pThis = nullptr;

			if (uMsg == Win32::Messages::NonClientCreate)
			{
				Win32::CREATESTRUCT* pCreate = (Win32::CREATESTRUCT*)lParam;
				pThis = (TWindow*)pCreate->lpCreateParams;
				Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData, (Win32::LONG_PTR)pThis);

				pThis->m_window = HwndUniquePtr(hwnd);
			}
			else
			{
				pThis = (TWindow*)GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData);
			}

			return pThis
				? pThis->HandleMessage(hwnd, uMsg, wParam, lParam)
				: Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
		}

		static constexpr std::array KnownMessages{
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
				Win32::LRESULT result;
				bool handled = (... or 
					[=, &self, &result]<typename TMsg = Win32Message<std::get<Is>(KnownMessages)>>()
					{
						if constexpr (Handles<decltype(self), TMsg>)
							return TMsg::uMsg == msgType ? (result = self.OnMessage(TMsg{ hwnd, wParam, lParam }), true) : false;
						return false;
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
