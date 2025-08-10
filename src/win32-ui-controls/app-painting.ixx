export module app:painting;
import std;
import :win32;
import :common;

export namespace UI
{
	struct PaintingContext
	{
		PaintingContext(Win32::HWND window, auto&& fn, auto&&...args)
		{
			Win32::PAINTSTRUCT ps;
			Win32::HDC dc = Win32::BeginPaint(window, &ps);
			std::invoke(fn, window, dc, ps, std::forward<decltype(args)>(args)...);
			Win32::EndPaint(window, &ps);
		}
	};

	using GdiObjectDeleter = Raii::IndirectUniquePtr<Win32::HBRUSH, Win32::DeleteObject>;
	struct ColoredBrush
	{
		ColoredBrush(std::byte r, std::byte g, std::byte b)
		{

		}
		GdiObjectDeleter m_brush;
	};
}
