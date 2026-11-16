export module CpuId:UI.Label;
import std;
import Win32;
import :Error;
import :UI.Control;

export namespace UI
{
    class Label : public Control<Label>
    {
    public:
        Label(
            unsigned controlId,
            const std::wstring& text,
            Win32::HWND parent,
            unsigned x,
            unsigned y,
            unsigned width,
            unsigned height,
            Win32::DWORD additionalStyles = 0
        )
        {
            if (not parent)
                throw std::invalid_argument{ "parent window handle is null" };
            Create(
                controlId, 
                L"STATIC", 
                text, 
                Win32::WindowStyles::Child | Win32::WindowStyles::Visible | additionalStyles, 
                parent, 
                x, y, 
                width, height, 
                additionalStyles);
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