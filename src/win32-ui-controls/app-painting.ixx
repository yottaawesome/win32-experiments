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

	using BrushDeleter = Raii::IndirectUniquePtr<Win32::HBRUSH, Win32::DeleteObject>;

	template<std::uint16_t R, std::uint16_t G, std::uint16_t B>
	struct ColoredBrush
	{
		constexpr auto Red() const noexcept -> std::uint16_t { return R; }
		constexpr auto Green() const noexcept -> std::uint16_t { return G; }
		constexpr auto Blue() const noexcept -> std::uint16_t { return B; }

		auto Get() const noexcept -> Win32::HBRUSH
		{
			static BrushDeleter Brush =
				[]
				{
					Win32::COLORREF color = Win32::RGB(R, G, B);
					return BrushDeleter{ Win32::CreateSolidBrush(color) };
				}();
			return Brush.get();
		}

		operator Win32::HBRUSH() const noexcept
		{
			return Get();
		}
	};

	constexpr ColoredBrush<255, 0, 0> RedBrush;
	constexpr ColoredBrush<0, 255, 0> GreenBrush;
	constexpr ColoredBrush<0, 0, 255> BlueBrush;

	using PenDeleter = Raii::IndirectUniquePtr<Win32::HPEN, Win32::DeleteObject>;
	template<std::uint16_t R, std::uint16_t G, std::uint16_t B>
	struct ColoredPen
	{
		constexpr auto Red() const noexcept -> std::uint16_t { return R; }
		constexpr auto Green() const noexcept -> std::uint16_t { return G; }
		constexpr auto Blue() const noexcept -> std::uint16_t { return B; }

		auto Get() const noexcept -> Win32::HPEN
		{
			static PenDeleter Brush =
				[]
				{
					Win32::COLORREF color = Win32::RGB(R, G, B);
					return PenDeleter{ Win32::CreatePen(Win32::PsInsideFrame, 0, color)};
				}();
			return Brush.get();
		}

		operator Win32::HPEN() const noexcept
		{
			return Get();
		}

		operator Win32::HGDIOBJ() const noexcept
		{
			return Get();
		}
	};

	constexpr ColoredPen<255, 0, 0> RedPen;
	constexpr ColoredPen<0, 255, 0> GreenPen;
	constexpr ColoredPen<0, 0, 255> BluePen;
}

namespace UI::StockObjects
{
	template<int VStockObject>
	struct StockObject
	{
		operator Win32::HGDIOBJ() const
		{
			return Win32::GetStockObject(VStockObject);
		}
	};

	constexpr StockObject<Win32::Brushes::Black> BlackBrush;
	constexpr StockObject<Win32::Pens::Black> WhitePen;
	constexpr StockObject<Win32::Pens::Black> BlackPen;
}
