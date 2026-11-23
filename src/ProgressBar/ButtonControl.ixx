export module MultiControlSamples:ButtonControl;
import std;
import Win32;
import :WindowCommon;
import :ControlWindow;

export namespace MultiControlSamples
{
	class Button
	{
	public:
		std::move_only_function<auto()->void> OnClick;
		
		Button(std::move_only_function<auto()->void> onClick)
			: OnClick{ std::move(onClick) }
		{ }

		constexpr auto GetDefaultParams(this auto&&) noexcept -> ControlParams
		{
			return ControlParams{
				.ClassName = Win32::WindowClasses::Button,
				.Style = Win32::Button::Styles::DefPushButton,
				.ExtendedStyle = 0
			};
		}

		auto HandleMessage(this auto&& self, Win32::HWND hwnd, Win32::UINT msg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
		{
			switch (msg)
			{
				case Win32::WindowMessages::ReflectedCommand:
				{
					self.OnClick ? self.OnClick() : void();
					return 0;
				}
			}
			return Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
		}
	};

	using ButtonControl = ControlWindow<Button>;
}