export module MultiControlSamples:StatusBarControl;
import std;
import Win32;
import :ControlWindow;
import :WindowCommon;

export namespace MultiControlSamples
{
	class StatusBar
	{
	public:
		StatusBar() = default;

		[[nodiscard]]
		constexpr auto GetDefaultParams(this auto&&) noexcept -> ControlParams
		{
			return ControlParams{
				.ClassName = Win32::WindowClasses::StatusBar,
				.Style = Win32::StatusBar::Styles::SizeGrip,
				.ExtendedStyle = 0
			};
		}

		auto Split(this auto&& self, std::same_as<int> auto...sizes) -> decltype(self)
		{
			if (auto hwnd = self.GetHwnd(); hwnd)
				Win32::SendMessageW(hwnd, Win32::StatusBar::Messages::SetParts, sizeof...(sizes), reinterpret_cast<Win32::LPARAM>(std::array{ sizes... }.data()));
			return self;
		}

		auto SetText(this auto&& self, int partIndex, std::wstring_view text) -> decltype(self)
		{
			if (auto hwnd = self.GetHwnd(); hwnd)
				Win32::SendMessageW(hwnd, Win32::StatusBar::Messages::SetText, partIndex, reinterpret_cast<Win32::LPARAM>(text.data()));
			return self;
		}

		auto Resize(this auto&& self) -> decltype(self)
		{
			if (auto hwnd = self.GetHwnd(); hwnd)
				Win32::SendMessageW(hwnd, Win32::WindowMessages::Size, 0, 0);
			return self;
		}

	protected:
		auto HandleMessage(this auto&& self, Win32::HWND hwnd, Win32::UINT msg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
		{
			return Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
		}
	};
	using StatusBarControl = ControlWindow<StatusBar>;
}