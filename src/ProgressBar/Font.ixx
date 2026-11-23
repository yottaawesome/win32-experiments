export module MultiControlSamples:Font;
import std;
import Win32;
import :Raii;
import :Error;

export namespace MultiControlSamples
{
	template<std::invocable CreateFn>
	struct Font
	{
		CreateFn Fn;

		auto Get() const -> Win32::HFONT
		{
			static auto font = FontUniquePtr{ Fn() };
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

	constexpr auto SegoeFont = Font{
		[] -> Win32::HFONT
		{
			auto font = Win32::CreateFontW(
				-32, // Font height (negative for desired height in logical units)
				0,   // Width (0 for default)
				0,   // Escapement
				0,   // Orientation
				Win32::FontWeight::Normal, // Font weight
				false,     // Italic
				false,     // Underline
				false,     // Strikeout
				Win32::DefaultCharset, // Character set
				Win32::OutPrecision::OutOutlinePrecision, // Output precision
				Win32::ClipPrecision::DefaultPrecision, // Clipping precision
				Win32::FontQuality::ClearType, // Quality
				Win32::Pitch::Variable | Win32::FontFamily::Swiss, // Pitch and family
				L"Segoe UI" // Font face name
			);
			if (not font)
				throw Error::Win32Error{ Win32::GetLastError(), "Failed to Segoe font." };
			return font;
		}
	};
}
