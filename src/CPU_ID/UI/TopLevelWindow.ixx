export module CpuId:UI.TopLevelWindow;
import std;
import Win32;
import :UI.Common;
import :UI.CustomWindow;

export namespace UI
{
    class TopLevelWindow : public CustomWindow
    {
    public:
        TopLevelWindow() = default;

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
}