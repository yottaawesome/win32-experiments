export module app:painting;
import std;
import :win32;
import :common;

export namespace UI
{
	struct SelectedObject
	{
		SelectedObject(Win32::HDC hdc, auto&& toSelect)
			: HDC(hdc)
		{
			if (HDC)
				Previous = Win32::SelectObject(HDC, toSelect.Get());
		}
		Win32::HDC HDC = nullptr;
		Win32::HGDIOBJ Previous = nullptr;
	};

	using BrushUniquePtr = Raii::IndirectUniquePtr<Win32::HBRUSH, Win32::DeleteObject>;
	using HrgnUniquePtr = Raii::IndirectUniquePtr<Win32::HRGN, Win32::DeleteObject>;

	template<std::uint16_t R, std::uint16_t G, std::uint16_t B>
	struct ColoredBrush
	{
		constexpr ColoredBrush()
		{
			if not consteval { Create(); }
		}

		constexpr auto Red() noexcept -> std::uint16_t { return R; }
		constexpr auto Green() noexcept -> std::uint16_t { return G; }
		constexpr auto Blue() noexcept -> std::uint16_t { return B; }

		void Create(this auto&& self)
		{
			Win32::COLORREF color = Win32::RGB(R, G, B);
			self.Brush = BrushUniquePtr{ Win32::CreateSolidBrush(color) };
		}

		auto Get(this auto&& self) noexcept -> Win32::HBRUSH
		{
			if (not self.Brush) 
				self.Create();
			return self.Brush.get();
		}

		operator Win32::HBRUSH(this auto&& self) noexcept
		{
			return self.Get();
		}

		mutable BrushUniquePtr Brush;
	};

	constexpr ColoredBrush<255, 0, 0> RedBrush;
	constexpr ColoredBrush<0, 255, 0> GreenBrush;
	constexpr ColoredBrush<0, 0, 255> BlueBrush;

	using PenDeleter = Raii::IndirectUniquePtr<Win32::HPEN, Win32::DeleteObject>;

	template<std::uint16_t R, std::uint16_t G, std::uint16_t B>
	struct ColoredPen
	{
		constexpr ColoredPen()
		{
			if not consteval { Create(); }
		}

		constexpr auto Red(this auto&&) noexcept -> std::uint16_t { return R; }
		constexpr auto Green(this auto&&) noexcept -> std::uint16_t { return G; }
		constexpr auto Blue(this auto&&) noexcept -> std::uint16_t { return B; }

		void Create(this auto&& self) 
		{
			Win32::COLORREF color = Win32::RGB(R, G, B);
			self.Pen = PenDeleter{ Win32::CreatePen(Win32::PsInsideFrame, 0, color) };
		}

		auto Get(this auto&& self) noexcept -> Win32::HPEN
		{
			if (not self.Pen)
				self.Create();
			return self.Pen.get();
		}

		operator Win32::HPEN(this auto&& self) noexcept
		{
			return self.Get();
		}

		operator Win32::HGDIOBJ(this auto&& self) noexcept
		{
			return self.Get();
		}

		mutable PenDeleter Pen;
	};

	constexpr ColoredPen<255, 0, 0> RedPen;
	constexpr ColoredPen<0, 255, 0> GreenPen;
	constexpr ColoredPen<0, 0, 255> BluePen;

	template<int VStockObject, typename TDummy, typename TConvertible = void>
	struct StockObject
	{
		operator Win32::HGDIOBJ(this auto&& self) noexcept 
			{ return self.Get(); }
		operator TConvertible(this auto&& self) noexcept requires (not (std::is_void_v<TConvertible>))
			{ return static_cast<TConvertible>(self.Get()); }
		auto Get(this auto&&) noexcept -> Win32::HGDIOBJ 
			{ return Win32::GetStockObject(VStockObject); }
	};

	template<int VStockObject>
	using StockBrush = StockObject<VStockObject, struct DummyBrush, Win32::HBRUSH>;
	template<int VStockObject>
	using StockPen = StockObject<VStockObject, struct DummyPen, Win32::HPEN>;

	constexpr StockBrush<Win32::Brushes::Black> BlackBrush;
	constexpr StockBrush<Win32::Brushes::White> WhiteBrush;
	constexpr StockPen<Win32::Pens::White> WhitePen;
	constexpr StockPen<Win32::Pens::Black> BlackPen;
}

namespace UI
{
	template<typename T>
	struct AnyBrushT : std::false_type {};
	template<std::uint16_t R, std::uint16_t G, std::uint16_t B>
	struct AnyBrushT<ColoredBrush<R, G, B>> : std::true_type {};
	template<int VStockObject>
	struct AnyBrushT<StockBrush<VStockObject>> : std::true_type {};
	template<typename T>
	constexpr bool AnyBrushV = AnyBrushT<T>::value;
	template<typename T>
	concept AnyBrush = AnyBrushV<std::remove_cvref_t<T>>;

	template<typename T>
	struct AnyPenT : std::false_type {};
	template<std::uint16_t R, std::uint16_t G, std::uint16_t B>
	struct AnyPenT<ColoredPen<R, G, B>> : std::true_type {};
	template<int VStockObject>
	struct AnyPenT<StockPen<VStockObject>> : std::true_type {};
	template<typename T>
	constexpr bool AnyPenV = AnyPenT<T>::value;
	template<typename T>
	concept AnyPen = AnyPenV<std::remove_cvref_t<T>>;

	template<typename T>
	concept PenOrBrush = AnyPen<T> or AnyBrush<T>;

	struct PaintContext
	{
		~PaintContext()
		{
			if (not Window or not HDC)
				return;
			// Restore defaults
			if (DefaultBrush)
				Win32::SelectObject(HDC, DefaultBrush);
			if (DefaultPen)
				Win32::SelectObject(HDC, DefaultPen);
			Win32::EndPaint(Window, &PS);
		}

		PaintContext(Win32::HWND window)
			: Window(window)
		{
			if (HDC = Win32::BeginPaint(Window, &PS); not HDC)
				throw Error::Win32Error(Win32::GetLastError());
		}

		auto Select(this auto&& self, PenOrBrush auto&& obj) -> decltype(auto)
		{
			Win32::HGDIOBJ previous = Win32::SelectObject(self.HDC, obj.Get());
			if constexpr (AnyBrush<decltype(obj)>)
				if (not self.DefaultBrush)
					self.DefaultBrush = previous;
			else if constexpr (AnyPen<decltype(obj)>)
				if (not self.DefaultPen)
					self.DefaultPen = previous;
			return std::forward_like<decltype(self)>(self);
		}

		auto DrawTextInClientRect(this auto&& self, std::wstring_view text, auto...flags) -> decltype(auto)
		{
			Win32::DrawTextW(
				self.HDC, 
				text.data(), 
				static_cast<Win32::DWORD>(text.size()), 
				&self.PS.rcPaint, 
				(flags | ...)
			);
			return std::forward_like<decltype(self)>(self);
		}

		auto RoundBorder(this auto&& self, int width, int height) -> decltype(auto)
		{
			Win32::RoundRect(
				self.HDC,
				self.PS.rcPaint.left,
				self.PS.rcPaint.top,
				self.PS.rcPaint.right,
				self.PS.rcPaint.bottom,
				width,
				height
			);
			return std::forward_like<decltype(self)>(self);
		}

		auto SetBackgroundMode(this auto&& self, Win32::DWORD mode) -> decltype(auto)
		{
			Win32::SetBkMode(self.HDC, mode);
			return std::forward_like<decltype(self)>(self);
		}

		auto SetTextColor(this auto&& self, std::uint8_t r, std::uint8_t g, std::uint8_t b) -> decltype(auto)
		{
			Win32::SetTextColor(self.HDC, Win32::RGB(r, g, b));
			return std::forward_like<decltype(self)>(self);
		}

		Win32::PAINTSTRUCT PS{};
		Win32::HDC HDC = nullptr;

		Win32::HGDIOBJ DefaultBrush = nullptr;
		Win32::HGDIOBJ DefaultPen = nullptr;
		Win32::HWND Window = nullptr;
	};
}
