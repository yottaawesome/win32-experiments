export module CpuId:UI.Layout;
import std;
import Win32;

export namespace UI
{
    class AnchorLayout
    {
    public:
		enum class Anchor
		{
			TopLeft,
			TopRight,
			BottomLeft,
			BottomRight,
            Center
		};
        struct Entry
        {
			Win32::HWND hwnd = nullptr;
			Anchor anchor = Anchor::TopLeft;
			unsigned OffsetX = 0;
			unsigned OffsetY = 0;
        };

		AnchorLayout(Win32::HWND reference)
			: m_reference(reference)
		{
			if (not reference)
				throw std::invalid_argument{ "reference window handle is null" };
		}

		auto Add(this auto&& self, Entry entry) -> decltype(self)
		{
			if (not entry.hwnd)
				throw std::invalid_argument{ "window handle is null" };
			self.m_toSize.push_back(entry);
			return self;
		}

        auto Apply(this auto&& self) -> decltype(self)
        {
			for (auto& entry : self.m_toSize)
			{
				auto referenceRect = Win32::RECT{};
				Win32::GetClientRect(self.m_reference, &referenceRect);
				auto entryRect = Win32::RECT{};
				Win32::GetWindowRect(entry.hwnd, &entryRect);
				auto width = static_cast<unsigned>(entryRect.right - entryRect.left);
				auto height = static_cast<unsigned>(entryRect.bottom - entryRect.top);
				auto x = 0;
				auto y = 0;
				switch (entry.anchor)
				{
				    case Anchor::TopLeft:
					    x = entry.OffsetX;
					    y = entry.OffsetY;
					    break;
				    case Anchor::TopRight:
					    x = (referenceRect.right - referenceRect.left) - width - entry.OffsetX;
					    y = entry.OffsetY;
					    break;
				    case Anchor::BottomLeft:
					    x = entry.OffsetX;
					    y = (referenceRect.bottom - referenceRect.top) - height - entry.OffsetY;
					    break;
				    case Anchor::BottomRight:
					    x = (referenceRect.right - referenceRect.left) - width - entry.OffsetX;
					    y = (referenceRect.bottom - referenceRect.top) - height - entry.OffsetY;
					    break;
				    case Anchor::Center:
					    x = ((referenceRect.right - referenceRect.left) / 2) - (width / 2) + entry.OffsetX;
					    y = ((referenceRect.bottom - referenceRect.top) / 2) - (height / 2) + entry.OffsetY;
					    break;
				}
				Win32::SetWindowPos(
					entry.hwnd, nullptr,
					static_cast<int>(x), static_cast<int>(y),
					static_cast<int>(width), static_cast<int>(height),
					Win32::SetWindowPosFlags::NoZOrder);
			}
			return self;
        }

    private:
		Win32::HWND m_reference = nullptr;
		std::vector<Entry> m_toSize;
    };

    class GridLayout
    {
    public:
        GridLayout(unsigned rows, unsigned columns, auto rowGap, auto columnGap)
            : m_rows(rows), m_columns(columns), m_rowGap(rowGap), m_columnGap(columnGap)
        {
            if (rows == 0 or columns == 0)
                throw std::invalid_argument{ "rows and columns must be greater than zero" };
        }

        auto AddWindow(
            this auto&& self,
            Win32::HWND hwnd,
            unsigned row,
            unsigned column,
            unsigned rowSpan = 1,
            unsigned columnSpan = 1
        ) -> decltype(self)
        {
            if (not hwnd)
                throw std::invalid_argument{ "window handle is null" };
            self.m_toSize.push_back({ hwnd, row, column, rowSpan, columnSpan });
            return self;
        }

        auto Apply(this auto&& self, Win32::HWND reference) -> decltype(self)
        {
            auto referenceRect = Win32::RECT{};
            Win32::GetClientRect(reference, &referenceRect);
			return self.Apply(referenceRect);
        }

        auto Apply(this auto&& self, const Win32::RECT& referenceRect) -> decltype(self)
        {
            auto height = static_cast<std::uint64_t>(referenceRect.bottom - referenceRect.top);
            auto width = static_cast<std::uint64_t>(referenceRect.right - referenceRect.left);
            auto totalRowGap = std::uint64_t{ self.m_rowGap } * (self.m_rows - 1);
            auto totalColumnGap = std::uint64_t{ self.m_columnGap } * (self.m_columns - 1);
            auto rowHeight = (height - std::min(height, totalRowGap)) / self.m_rows;
            auto colWidth = (width - std::min(width, totalColumnGap)) / self.m_columns;

            for (auto& wnd : self.m_toSize)
            {
                auto x = (colWidth + self.m_columnGap) * wnd.column;
                auto y = (rowHeight + self.m_rowGap) * wnd.row;
                Win32::SetWindowPos(
                    wnd.hwnd, nullptr,
                    static_cast<int>(x), static_cast<int>(y),
                    static_cast<int>(colWidth), static_cast<int>(rowHeight),
                    Win32::SetWindowPosFlags::NoZOrder);
            }
            return self;
        }

    private:
        unsigned m_rows = 1;
        unsigned m_columns = 1;
        unsigned m_rowGap = 0;
        unsigned m_columnGap = 0;
        struct Position
        {
            Win32::HWND hwnd = nullptr;
            unsigned row = 0;
            unsigned column = 0;
            unsigned rowSpan = 1;
            unsigned columnSpan = 1;
        };
        std::vector<Position> m_toSize;
    };
}
