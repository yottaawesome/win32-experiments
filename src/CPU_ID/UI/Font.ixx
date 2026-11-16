export module CpuId:UI.Font;
import std;
import Win32;
import :Error;
import :Raii;

export namespace UI
{
    template<std::invocable CreateFn>
    struct Font
    {
        CreateFn Fn;

        auto Get() const -> Win32::HFONT
        {
            static auto font = Raii::FontUniquePtr{ Fn() };
            return font.get();
        }

        operator Win32::HFONT() const
        {
            return Get();
        }
    };

    constexpr auto SystemFont = Font{
        [] -> Win32::HFONT
        {
            auto metrics = Win32::NONCLIENTMETRICS{ .cbSize = sizeof(Win32::NONCLIENTMETRICS) };
            Win32::SystemParametersInfoW(Win32::SpiGetNonClientMetrics, metrics.cbSize, &metrics, 0);
            // When you're done with the font, don't forget to call Win32::DeleteObject();
            auto font = Win32::CreateFontIndirectW(&metrics.lfCaptionFont);
            if (not font)
                throw Error::Win32Error{ Win32::GetLastError(), "Failed to load system font." };
            return font;
        }
    };
}