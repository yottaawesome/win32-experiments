export module MultiControlSamples:Panel;
import std;
import Win32;
import :Error;
import :WindowCommon;
import :CustomWindow;

export namespace MultiControlSamples
{
	class Panel
	{
	public:
		Panel() = default;

		//
		// No copy semantics
		Panel(const Panel&) = delete;
		auto operator=(const Panel&) -> Panel & = delete;

		//
		// Move semantics
		Panel(Panel&& other) noexcept = default;
		auto operator=(Panel&& other) noexcept -> Panel & = default;

		//
		// Public methods
	public:
		[[nodiscard]]
		constexpr auto GetDefaultParams(this auto&&) noexcept -> CustomWindowParams
		{
			constexpr auto Style = 
				Win32::WindowStyles::Child 
				| Win32::WindowStyles::Visible 
				// ClipChildren is important for the panel to avoid drawing over its child controls
				| Win32::WindowStyles::ClipChildren;
			return {
				.ClassName = L"CustomPanel",
				.Style = Style,
			};
		}

		auto SetFontOnHierarchy(this auto&& self, Win32::HFONT font) -> decltype(self)
		{
			if (not self.GetHwnd())
				return self;
			if (not font)
				throw Error::RuntimeError{ "Invalid font handle passed to SetFontOnHierarchy()" };
			Win32::SendMessageW(
				self.GetHwnd(),
				Win32::WindowMessages::SetFont,
				reinterpret_cast<Win32::WPARAM>(font),
				true
			);
			Win32::EnumChildWindows(
				self.GetHwnd(),
				[](Win32::HWND child, Win32::LPARAM fontValue) -> Win32::BOOL
				{
					Win32::SendMessageW(
						child,
						Win32::WindowMessages::SetFont,
						static_cast<Win32::WPARAM>(fontValue),
						true
					);
					return true;
				},
				reinterpret_cast<Win32::LPARAM>(font)
			);
			return self;
		}

		auto ResizeToParent(
			this auto&& self, 
			unsigned paddingTop = 0, 
			unsigned paddingRight = 0,
			unsigned paddingBottom = 0,
			unsigned paddingLeft = 0
		) -> decltype(self)
		{
			if (not self.GetHwnd())
				return self;
			if (auto parent = Win32::GetParent(self.GetHwnd()); parent)
			{
				auto rect = Win32::RECT{};
				Win32::GetClientRect(parent, &rect);
				rect.left += static_cast<LONG>(paddingLeft);
				rect.top += static_cast<LONG>(paddingTop);
				rect.right -= static_cast<LONG>(paddingRight);
				rect.bottom -= static_cast<LONG>(paddingBottom);
				Win32::SetWindowPos(
					self.GetHwnd(),
					nullptr,
					0,
					0,
					std::max(0L, rect.right - rect.left),
					std::max(0L, rect.bottom - rect.top),
					Win32::SetWindowPosFlags::NoZOrder | Win32::SetWindowPosFlags::NoActivate
				);
			}
			return self;
		}

	protected:
		auto HandleMessage(
			this auto&& self,
			Win32::HWND hwnd,
			Win32::UINT msg,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam
		) -> Win32::LRESULT
		{
			switch (msg)
			{
				case Win32::WindowMessages::Command:
				{
					if (auto originatingWindow = reinterpret_cast<Win32::HWND>(lParam); originatingWindow)
					{
						Win32::SendMessageW(originatingWindow, Win32::WindowMessages::ReflectedCommand, wParam, lParam);
						return 0;
					}
					return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
				}

				case Win32::WindowMessages::Notify:
				{
					if (auto notification = reinterpret_cast<const Win32::NMHDR*>(lParam); notification and notification->hwndFrom)
						return Win32::SendMessageW(notification->hwndFrom, Win32::WindowMessages::ReflectedNotify, wParam, lParam);
					return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
				}

				default:
				{
					return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
				}
			}
		}
	};

	using PanelWindow = CustomWindow<Panel>;
}
