
export module ProgressBarSamples:TopLevelWindow;
import std;
import Win32;
import :Error;
import :WindowCommon;
import :CustomWindow;

export namespace ProgressBarSamples
{
	class TopLevelWindow
	{
	public:
		TopLevelWindow() = default;

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
		consteval auto GetWindowClassName(this auto&&) noexcept -> std::wstring_view
		{
			return L"ProgressBarSamplesMainWindow";
		}

		[[nodiscard]]
		consteval auto GetDefaultStyle(this auto&&) noexcept -> Win32::DWORD
		{
			return Win32::WindowStyles::WsOverlappedWindow | Win32::WindowStyles::WsVisible;
		}

		[[nodiscard]]
		consteval auto GetDefaultExtendedStyle(this auto&&) noexcept -> Win32::DWORD
		{
			return 0;
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
	};

}