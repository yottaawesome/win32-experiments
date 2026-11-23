export module MultiControlSamples:ProgressBarControl;
import std;
import Win32;
import :WindowCommon;
import :Error;
import :ControlWindow;

export namespace MultiControlSamples
{
	class ProgressBarBase
	{
	public:
		[[nodiscard]]
		constexpr auto GetDefaultParams(this auto&&) noexcept -> ControlParams
		{
			return ControlParams{
				.ClassName = Win32::WindowClasses::ProgressBar,
				.Style = Win32::ProgressBar::Styles::Smooth,
				.ExtendedStyle = 0
			};
		}

		auto SetRange(this auto&& self, unsigned min, unsigned max) noexcept -> decltype(self)
		{
			if (auto hwnd = self.GetHwnd(); hwnd)
				Win32::SendMessageW(hwnd, Win32::ProgressBar::Messages::SetRange, min, max);
			return self;
		}

		auto SetPosition(this auto&& self, int position) noexcept -> decltype(self)
		{
			if (auto hwnd = self.GetHwnd(); hwnd)
				Win32::SendMessageW(hwnd, Win32::ProgressBar::Messages::SetPos, position, 0);
			return self;
		}

		auto SetIncrement(this auto&& self, int increment) noexcept -> decltype(self)
		{
			if (auto hwnd = self.GetHwnd(); hwnd)
				Win32::SendMessageW(hwnd, Win32::ProgressBar::Messages::SetStep, increment, 0);
			return self;
		}

		auto AdvanceIncrement(this auto&& self) noexcept -> decltype(self)
		{
			if (auto hwnd = self.GetHwnd(); hwnd)
				Win32::SendMessageW(hwnd, Win32::ProgressBar::Messages::StepIt, 0, 0);
			return self;
		}

		// Marquee is an indeterminate progress bar style that shows a continuous scrolling block to indicate activity.
		auto SetMarquee(this auto&& self, bool enable, unsigned updateInterval) noexcept -> decltype(self)
		{
			if (auto hwnd = self.GetHwnd(); hwnd)
			{
				// The marquee animation requires the PBS_MARQUEE style to be set on the progress bar control. 
				// We can set or clear this style using SetWindowLongPtrW.
				const auto style = Win32::GetWindowLongPtrW(hwnd, Win32::Gwl::Style);
				constexpr auto marquee = Win32::ProgressBar::Styles::Marquee;
				if (enable)
				{
					Win32::SetWindowLongPtrW(hwnd, Win32::Gwl::Style, style | marquee);
					Win32::SendMessageW(hwnd, Win32::ProgressBar::Messages::SetMarquee, 1, updateInterval);
				}
				else
				{
					Win32::SendMessageW(hwnd, Win32::ProgressBar::Messages::SetMarquee, 0, 0);
					Win32::SetWindowLongPtrW(hwnd, Win32::Gwl::Style, style & ~marquee);
				}
			}
			return self;
		}

		auto SetState(this auto&& self, int state) noexcept -> decltype(self)
		{
			if (auto hwnd = self.GetHwnd(); hwnd)
				Win32::SendMessageW(hwnd, Win32::ProgressBar::Messages::SetState, state, 0);
			return self;
		}

		auto SetNormalState(this auto&& self) noexcept -> decltype(self)
		{
			return self.SetState(Win32::ProgressBar::States::Normal);
		}

		auto SetErrorState(this auto&& self) noexcept -> decltype(self)
		{
			return self.SetState(Win32::ProgressBar::States::Error);
		}

		auto SetPausedState(this auto&& self) noexcept -> decltype(self)
		{
			return self.SetState(Win32::ProgressBar::States::Paused);
		}

		[[nodiscard]]
		auto GetRange(this auto&& self) noexcept -> std::pair<int, int>
		{
			auto range = std::pair<int, int>{};
			if (auto hwnd = self.GetHwnd(); hwnd)
				Win32::SendMessageW(hwnd, Win32::ProgressBar::Messages::GetRange, reinterpret_cast<Win32::WPARAM>(&range.first), reinterpret_cast<Win32::LPARAM>(&range.second));
			return range;
		}

		[[nodiscard]]
		auto GetPosition(this auto&& self) noexcept -> int
		{
			if (auto hwnd = self.GetHwnd(); hwnd)
				return static_cast<int>(Win32::SendMessageW(hwnd, Win32::ProgressBar::Messages::GetPos, 0, 0));
			return 0;
		}

	protected:
		auto HandleMessage(this auto&& self, Win32::HWND hwnd, Win32::UINT msg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
		{
			return Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
		}
	};

	using ProgressBarControl = ControlWindow<ProgressBarBase>;
}
