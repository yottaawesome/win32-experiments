
export module MultiControlSamples:TopLevelWindow;
import std;
import Win32;
import :Error;
import :WindowCommon;
import :CustomWindow;

export namespace MultiControlSamples
{
	struct TopLevelWindowCallbacks
	{
		std::move_only_function<auto(unsigned, unsigned)->void> OnResized;
	};

	class TopLevelWindow
	{
	public:
		TopLevelWindow(TopLevelWindowCallbacks callbacks) 
			: m_callbacks(std::move(callbacks)) 
		{}

		//
		// No copy semantics
		TopLevelWindow(const TopLevelWindow&) = delete;
		auto operator=(const TopLevelWindow&) -> TopLevelWindow & = delete;

		//
		// Move semantics
		TopLevelWindow(TopLevelWindow&& other) noexcept = default;
		auto operator=(TopLevelWindow&& other) noexcept -> TopLevelWindow& = default;

		//
		// Public methods
	public:
		[[nodiscard]]
		constexpr auto GetDefaultParams(this auto&&) noexcept -> CustomWindowParams
		{
			return {
				.ClassName = std::wstring{ L"ProgressBarSamplesMainWindow" },
				.Style = Win32::WindowStyles::WsOverlappedWindow | Win32::WindowStyles::WsVisible,
				.ExtendedStyle = 0
			};
		}

		constexpr auto SetCallbacks(this auto&& self, TopLevelWindowCallbacks callbacks) noexcept -> decltype(self)
		{
			self.m_callbacks = std::move(callbacks);
			return self;
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
				// Reflect command and notify messages back to the originating window
				case Win32::WindowMessages::Command:
				case Win32::WindowMessages::Notify:
				{
					auto originatingWindow = reinterpret_cast<Win32::HWND>(lParam);
					Win32::SendMessageW(originatingWindow, Win32::WindowMessages::ReflectedCommand, wParam, lParam);
					return 0;
				}

				// End the application when the main window is destroyed
				case Win32::WindowMessages::Destroy:
				{
					Win32::PostQuitMessage(0);
					return 0;
				}

				case Win32::WindowMessages::ExitSizeMove:
				{
					auto rect = Win32::RECT{};
					Win32::GetClientRect(hwnd, &rect);
					if (self.m_callbacks.OnResized)
						self.m_callbacks.OnResized(static_cast<unsigned>(rect.right - rect.left), static_cast<unsigned>(rect.bottom - rect.top));
					return 0;
				}

				case Win32::WindowMessages::Paint:
				{
					auto ps = Win32::PAINTSTRUCT{};
					auto hdc = Win32::BeginPaint(hwnd, &ps);
					Win32::EndPaint(hwnd, &ps);
					return 0;
				}

				default:
				{
					return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
				}
			}
		}

	protected:
		TopLevelWindowCallbacks m_callbacks;
	};
}