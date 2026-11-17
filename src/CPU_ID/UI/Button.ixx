export module CpuId:UI.Button;
import std;
import Win32;
import :Error;
import :UI.Control;
import :UI.Functions;

export namespace UI
{
	struct ButtonArgs
	{
		ControlArgs Control{};
		std::move_only_function<auto()-> void> OnClick;
	};

	class Button : public Control<Button>
	{
	public:
		~Button()
		{
			Destroy();
		}

		Button(ButtonArgs args) 
			: OnClick{ std::move(args.OnClick) }
		{
			if (not args.Control.Parent)
				throw std::invalid_argument{ "parent window handle is null" };
			Create(L"BUTTON", args.Control);
		}

		//
		// Copy
		Button(const Button&) = delete;
		auto operator=(const Button&) -> Button & = delete;

		std::move_only_function<auto()-> void> OnClick;

		[[nodiscard]]
		auto HandleMessage(
			Win32::HWND hwnd,
			unsigned uMsg,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam
		) -> std::optional<Win32::LRESULT>
		{
			switch (uMsg)
			{
				case Win32::Messages::ReflectedCommand:
				{
					if (Win32::HiWord(wParam) == Win32::BN::Clicked and OnClick)
						OnClick();
					return 0;
				}

				case Win32::Messages::Destroy:
				{
					release();
					return 0;
				}

				default:
					return std::nullopt;
			}
		}
	};
}