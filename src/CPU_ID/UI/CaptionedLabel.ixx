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
            : Caption{ 
                LabelArgs{ 
                    .Control = ControlArgs{ 
                        .Text = caption, 
                        .Parent = parent, 
                        .X = 0, .Y = 0, 
                        .Width = 100, .Height = 30, 
                        .WindowStyles = Win32::WindowStyles::Visible | Win32::SS::Right 
                    } 
                } }
            , Value{ 
                LabelArgs{ 
                    .Control = ControlArgs{ 
                        .Text = value, 
                        .Parent = parent, 
                        .X = 0, .Y = 0, 
                        .Width = 100, .Height = 30, 
                        .WindowStyles = Win32::WindowStyles::Visible | Win32::SS::Left 
                    } 
                } }
        {}

        CaptionedLabel(const std::wstring& caption, bool value, Win32::HWND parent)
            : Caption{ 
                LabelArgs{ 
                    .Control = ControlArgs{ 
                        .Text = caption, 
                        .Parent = parent, 
                        .X = 0, .Y = 0, 
                        .Width = 100, .Height = 30, 
                        .WindowStyles = Win32::WindowStyles::Visible | Win32::SS::Right } } }
            , Value{ 
                LabelArgs{ 
                    .Control = ControlArgs{ 
                        .Text = value ? L"true" : L"false", 
                        .Parent = parent, 
                        .X = 0, .Y = 0, 
                        .Width = 100, .Height = 30, 
                        .WindowStyles = Win32::WindowStyles::Visible | Win32::SS::Left } } }
        {}
    };
}
