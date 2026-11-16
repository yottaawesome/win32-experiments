export module CpuId:UI.Button;
import std;
import Win32;
import :Error;
import :UI.Control;

export namespace UI
{
	class Button : public Control<Button>
	{
	public:
		~Button()
		{
			Destroy();
		}

		Button() = default;

		Button(
			unsigned controlId,
			const std::wstring& text,
			Win32::HWND parent,
			unsigned x,
			unsigned y,
			unsigned width,
			unsigned height,
			std::move_only_function<auto()-> void> onClick,
			Win32::DWORD additionalStyles = 0
		) : OnClick{ std::move(onClick) }
		{
			if (not parent)
				throw std::invalid_argument{ "parent window handle is null" };
			Create(
				controlId, 
				L"BUTTON",
				text,
				Win32::WindowStyles::Child | Win32::WindowStyles::Visible | additionalStyles,
				parent, 
				x, y, 
				width, height, 
				additionalStyles
			);
		}

		//
		// Copy
		Button(const Button&) = delete;
		auto operator=(const Button&) -> Button & = delete;

		//
		// Move
		Button(Button&& other) noexcept = default;
		auto operator=(Button&& other) noexcept -> Button & = default;

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
					if (Win32::HiWord(wParam) == Win32::BN::Clicked)
					{
						if (OnClick)
							OnClick();
					}
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