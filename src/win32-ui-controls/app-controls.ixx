export module app:controls;
import :common;
import :win32;

export namespace UI
{
	struct ControlProperties
	{
		Win32::DWORD Id;
		std::wstring Class;
		std::wstring Text;
		Win32::DWORD Styles = 0;
		Win32::DWORD ExtendedStyles = 0;
		int X = 0;
		int Y = 0;
		int Width = 0;
		int Height = 0;
	};

	constexpr std::array HandledControlMessages{
		Win32::Messages::LeftButtonUp,
		Win32::Messages::Paint,
		Win32::Messages::Notify,
		Win32::Messages::DrawItem,
		Win32::Messages::MouseHover,
		Win32::Messages::MouseLeave,
		Win32::Messages::MouseMove,
		Win32::Messages::EraseBackground
	};

	struct Control : Window
	{
		auto Create(this auto&& self, Win32::HWND parent) -> void
		{
			static_assert(
				requires { { self.ClassName.data() } -> std::same_as<const wchar_t*>; },
				"This type needs a ClassName member that specifies the Win32 class name!");

			auto properties = self.GetDefaultProperties();
			Win32::HWND window = Win32::CreateWindowExW(
				properties.ExtendedStyles,
				self.ClassName.data(),
				properties.Text.empty() ? nullptr : properties.Text.data(),
				properties.Styles,
				properties.X,
				properties.Y,
				properties.Width,
				properties.Height,
				parent,
				(Win32::HMENU)(Win32::UINT_PTR)(properties.Id),
				Win32::GetModuleHandleW(nullptr),
				nullptr
			);
			if (self.m_window = HwndUniquePtr(window); not self.m_window)
				throw Error::Win32Error(Win32::GetLastError(), "Failed creating button.");
			if (not Win32::SetWindowSubclass(self.m_window.get(), SubclassProc<std::remove_cvref_t<decltype(self)>>, self.GetSubclassId(), reinterpret_cast<Win32::DWORD_PTR>(&self)))
				throw Error::Win32Error(Win32::GetLastError(), "Failed creating button.");

			if constexpr (requires { self.Init(); })
				self.Init();
		}

		auto HandleMessage(
			this auto&& self,
			Win32::HWND hwnd,
			Win32::UINT msg,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam,
			Win32::UINT_PTR uIdSubclass,
			Win32::DWORD_PTR dwRefData
		) -> Win32::LRESULT
		{
			return[&self, hwnd, msg, wParam, lParam]<size_t...Is>(std::index_sequence<Is...>)
			{
				Win32::LRESULT result = 0;
				bool handled = ((
					std::get<Is>(HandledControlMessages) == msg
						? (result = self.OnMessage(Win32Message<std::get<Is>(HandledControlMessages)>{ hwnd, wParam, lParam }), true)
						: false
				) or ...);
				return handled ? result : self.OnMessage(GenericWin32Message{ .Hwnd = hwnd, .uMsg = msg, .wParam = wParam, .lParam = lParam });
			}(std::make_index_sequence<HandledControlMessages.size()>());
		}

		auto OnMessage(this Control& self, auto msg) noexcept -> Win32::LRESULT
		{
			return Win32::DefSubclassProc(msg.Hwnd, msg.uMsg, msg.wParam, msg.lParam);
		}

		template<typename TControl>
		static auto SubclassProc(
			Win32::HWND hwnd,
			Win32::UINT msg,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam,
			Win32::UINT_PTR idSubclass,
			Win32::DWORD_PTR refData
		) -> Win32::LRESULT
		{
			TControl* pThis = reinterpret_cast<TControl*>(refData);
			return pThis
				? pThis->HandleMessage(hwnd, msg, wParam, lParam, idSubclass, refData)
				: Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
		}

		auto GetId(this const auto& self) noexcept -> unsigned { return self.GetDefaultProperties().Id; }
	};

	struct Button : Control
	{
		using Control::OnMessage;
		
		constexpr static std::wstring_view ClassName = L"Button";

		auto OnMessage(this auto&& self, Win32Message<Win32::Messages::LeftButtonUp> msg) -> Win32::LRESULT
		{
			if constexpr (requires { self.OnClick(); })
				self.OnClick();
			return Win32::DefSubclassProc(msg.Hwnd, msg.uMsg, msg.wParam, msg.lParam);
		}

		auto GetDefaultProperties(this auto&& self) -> ControlProperties
		{
			return {
				.Id = 100,
				.Class = L"Button",
				.Text = L"", // window text
				.Styles = Win32::Styles::PushButton | Win32::Styles::Child | Win32::Styles::Visible,
				.X = 10,
				.Y = 10,
				.Width = 100,
				.Height = 50
			};
		};

		void Click(this auto&& self) noexcept
		{
			if (self.GetHandle())
				Win32::SendMessageW(self.GetHandle(), Win32::Messages::ButtonClick, 0, 0);
		}
	};

	struct OwnerDrawnButton : Button, MouseTracking<true>
	{
		using Button::ClassName;
		using Control::OnMessage;
		using MouseTracking::OnMessage;

		auto GetDefaultProperties(this auto&& self) -> ControlProperties
		{
			return {
				.Id = 100,
				.Class = L"Button",
				.Text = L"Button", // window text
				.Styles = Win32::Styles::ButtonOwnerDrawn | Win32::Styles::Child | Win32::Styles::Visible,
				.X = 10,
				.Y = 10,
				.Width = 300,
				.Height = 50
			};
		};

		auto OnMessage(this auto&& self, Win32Message<Win32::Messages::Paint> msg) -> Win32::LRESULT
		{
			Win32::PAINTSTRUCT ps;
			Win32::HDC hdc = Win32::BeginPaint(msg.Hwnd, &ps);

			auto oldBrush = Win32::SelectObject(hdc, Win32::GetStockObject(Win32::Brushes::Black));
			Win32::RoundRect(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, 5, 5);
			//Win32::FillRect(hdc, &ps.rcPaint, (Win32::HBRUSH)(Win32::Color::Window + 1));

			std::wstring_view message = self.MouseHovering ? L"Hover!" : L"No hover!";

			Win32::SetBkMode(hdc, Win32::BackgroundMode::Transparent);
			Win32::SetTextColor(hdc, Win32::RGB(255,255,255));
			Win32::DrawTextW(
				hdc,
				message.data(),
				static_cast<Win32::DWORD>(message.size()),
				&ps.rcPaint,
				Win32::DrawTextOptions::Center | Win32::DrawTextOptions::VerticalCenter | Win32::DrawTextOptions::SingleLine
			);
			/*Win32::TextOutW(
				hdc, 
				rc.right/2, 
				rc.bottom/2-7, 
				message.data(),
				static_cast<Win32::DWORD>(message.size())
			);*/

			Win32::SelectObject(hdc, oldBrush);
			Win32::EndPaint(msg.Hwnd, &ps);
			return 0;
		}

		auto GetSubclassId(this auto&& self) noexcept { return 1; }
	};
}
