export module CpuId:UI.Panel;
import std;
import Win32;
import :UI.WindowClass;
import :UI.Common;
import :UI.CustomWindow;

export namespace UI
{
    class Panel : public CustomWindow
    {
    public:
		~Panel()
		{
            Destroy();
		}

        Panel() = default;

        auto HandleMessage(
            Win32::HWND hwnd,
            unsigned uMsg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam
        ) -> std::optional<Win32::LRESULT>
        {
            switch (uMsg)
            {
			    case Win32::Messages::Command:
			    {
					auto originalHwnd = reinterpret_cast<Win32::HWND>(lParam);
					return Win32::SendMessageW(
						originalHwnd,
						Win32::Messages::ReflectedCommand,
						wParam,
						lParam
					);
			    }

                case Win32::Messages::CtlColorStatic:
                {
                    return reinterpret_cast<Win32::LRESULT>(Win32::GetStockObject(Win32::Brushes::White));
                }

                default:
                    return std::nullopt;
            }
        }

        auto SetFontOnChildren(Win32::HFONT font)
        {
            Win32::EnumChildWindows(
                GetHwnd(),
                [](Win32::HWND child, Win32::LPARAM fontValue) -> Win32::BOOL
                {
                    Win32::SendMessageW(
                        child,
                        Win32::Messages::SetFont,
                        static_cast<Win32::WPARAM>(fontValue),
                        true
                    );
                    return true;
                },
                reinterpret_cast<Win32::LPARAM>(font)
            );
        }
    };

	using PanelClass = WindowClass<
        L"PanelWindowClass", 
        Panel, 
        false, 
        Win32::WindowStyles::Child | Win32::WindowStyles::ClipChildren>;
}