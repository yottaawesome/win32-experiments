export module CpuId:UI.Label;
import std;
import Win32;
import :Error;
import :UI.Control;

export namespace UI
{
	struct LabelArgs
	{
		ControlArgs Control{};
	};

    class Label : public Control<Label>
    {
    public:
        Label(LabelArgs args)
        {
            if (not args.Control.Parent)
                throw std::invalid_argument{ "parent window handle is null" };
            Create(L"STATIC", args.Control);
        }

        auto HandleMessage(Win32::HWND hwnd, unsigned uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> std::optional<Win32::LRESULT>
        {
            if (uMsg == Win32::Messages::NonClientDestroy)
            {
                Win32::RemoveWindowSubclass(hwnd, &Control<Label>::SubclassProc, 0);
                m_hwnd = nullptr;
            }
            return std::nullopt;
        }

        auto SetText(const std::wstring& text)
        {
            if (not m_hwnd)
                throw std::runtime_error{ "Label window handle is null" };
            Win32::SetWindowTextW(m_hwnd, text.c_str());
        }

        auto SetText(bool value)
        {
            if (not m_hwnd)
                throw std::runtime_error{ "Label window handle is null" };
            Win32::SetWindowTextW(m_hwnd, value ? L"True" : L"False");
        }
    };
}