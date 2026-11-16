export module CpuId:UI.CaptionedLabel;
import std;
import Win32;
import :UI.Functions;
import :UI.Label;

export namespace UI
{
    struct CaptionedLabel
    {
        UI::Label Caption;
        UI::Label Value;

        CaptionedLabel(const std::wstring& caption, const std::wstring& value, Win32::HWND parent)
            : Caption{ UI::NextControlId(), caption, parent, 0, 0, 100, 30, Win32::SS::Right }
            , Value{ UI::NextControlId(), value, parent, 0, 0, 100, 30, Win32::SS::Left }
        {}

        CaptionedLabel(const std::wstring& caption, bool value, Win32::HWND parent)
            : Caption{ UI::NextControlId(), caption, parent, 0, 0, 100, 30, Win32::SS::Right }
            , Value{ UI::NextControlId(), value ? L"true" : L"false", parent, 0, 0, 100, 30, Win32::SS::Left }
        {}
    };
}
