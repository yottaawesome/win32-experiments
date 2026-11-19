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

    struct Label : public ControlWindow<Label>
    {
        Label(LabelArgs args)
        {
            if (not args.Control.Parent)
                throw std::invalid_argument{ "parent window handle is null" };
            Create(Win32::WindowClasses::Static, args.Control);
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