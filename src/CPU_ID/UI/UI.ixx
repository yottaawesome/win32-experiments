export module CpuId:UI;
import std;
import Win32;
import :Error;
import :Raii;
import :Strings;

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

    class Panel
    {
    public:

    private:

    };

    class TopLevelWindow
    {
    public:
        ~TopLevelWindow()
        {
            if (m_hwnd)
            {
                auto tmp = m_hwnd;
                release();
                Win32::DestroyWindow(tmp);
            }
        }

        TopLevelWindow(Win32::HWND hwnd)
            : m_hwnd(hwnd)
        {}

        auto HandleMessage(
            Win32::HWND hwnd,
            unsigned uMsg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam
        ) -> std::optional<Win32::LRESULT>
        {
            switch (uMsg)
            {
            case Win32::Messages::CtlColorStatic:
            {
                return reinterpret_cast<Win32::LRESULT>(Win32::GetStockObject(Win32::Brushes::White));
            }
            default:
                return std::nullopt;
            }
        }

        void release()
        {
            if (m_hwnd)
            {
                Win32::SetWindowLongPtrW(m_hwnd, Win32::Gwlp::UserData, 0);
                m_hwnd = nullptr;
            }
        }

        auto GetHwnd() const noexcept -> Win32::HWND
        {
            return m_hwnd;
        }

        auto SetFontOnChildren(Win32::HFONT font)
        {
            Win32::EnumChildWindows(
                GetHwnd(),
                [](Win32::HWND child, Win32::LPARAM fontValue) -> Win32::BOOL
                {
                    Win32::SendMessageW(
                        child,
                        Win32::Messages::SetFont,
                        static_cast<Win32::WPARAM>(fontValue),
                        true
                    );
                    return true;
                },
                reinterpret_cast<Win32::LPARAM>(font)
            );
        }

    private:
        Win32::HWND m_hwnd = nullptr;
    };

    template<Strings::FixedString ClassName, typename TWindow>
    struct WindowClass
    {
    public:
        ~WindowClass()
        {
            Unregister();
        }

        WindowClass()
        {
            Register();
        }

        auto CreateWindow(
            const std::wstring& title,
            Win32::DWORD styles,
            Win32::DWORD exStyles,
            unsigned width,
            unsigned height
        ) const -> std::unique_ptr<TWindow>
        {
            auto hInstance = Win32::GetModuleHandleW(nullptr);
            if (not hInstance)
                throw Error::Win32Error{ Win32::GetLastError(), "GetModuleHandleW failed" };

            auto hwnd = Win32::CreateWindowExW(
                exStyles,
                ClassName.ToView().data(),
                title.c_str(),
                styles,
                0, 0,
                width, height,
                nullptr,
                nullptr,
                hInstance,
                nullptr);
            if (not hwnd)
                throw Error::Win32Error{ Win32::GetLastError(), "CreateWindowExW failed" };
            return std::unique_ptr<TWindow>(reinterpret_cast<TWindow*>(Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp::UserData)));
        }

    private:
        auto Unregister() const -> void
        {
            if (not Win32::UnregisterClassW(ClassName.ToView().data(), Win32::GetModuleHandleW(nullptr)))
                throw Error::Win32Error{ Win32::GetLastError(), "UnregisterClassW failed" };
        }

        auto Register() const -> void
        {
            auto hInstance = Win32::GetModuleHandleW(nullptr);
            if (not hInstance)
                throw Error::Win32Error{ Win32::GetLastError(), "GetModuleHandleW failed" };
            auto wc = Win32::WNDCLASSEXW{
                .cbSize = sizeof(Win32::WNDCLASSEXW),
                .style = 0,
                .lpfnWndProc = WndProc,
                .cbClsExtra = 0,
                .cbWndExtra = 0,
                .hInstance = hInstance,
                .hIcon = nullptr,
                .hCursor = nullptr,
                .hbrBackground = static_cast<Win32::HBRUSH>(Win32::GetStockObject(Win32::Brushes::White)),
                .lpszMenuName = nullptr,
                .lpszClassName = ClassName.ToView().data(),
                .hIconSm = nullptr
            };
            if (not Win32::RegisterClassExW(&wc))
                throw Error::Win32Error{ Win32::GetLastError(), "RegisterClassExW failed" };
        }

    private:
        static auto WndProc(Win32::HWND hwnd, unsigned uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
        {
            auto pThis = static_cast<TWindow*>(nullptr);
            if (uMsg == Win32::Messages::NonClientCreate)
            {
                auto* pCreate = reinterpret_cast<Win32::CREATESTRUCT*>(lParam);
                pThis = reinterpret_cast<TWindow*>(pCreate->lpCreateParams);
                if (not pThis)
                    pThis = new TWindow{ hwnd };
                Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp::UserData, reinterpret_cast<Win32::LONG_PTR>(pThis));
            }
            else
            {
                pThis = reinterpret_cast<TWindow*>(Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp::UserData));
                if (pThis and uMsg == Win32::Messages::NonClientDestroy)
                {
                    pThis->release();
                    Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp::UserData, 0);
                }
            }

            if (pThis)
            {
                auto result = pThis->HandleMessage(hwnd, uMsg, wParam, lParam);
                if (result and uMsg != Win32::Messages::NonClientDestroy)
                {
                    return *result;
                }
            }

            if (uMsg == Win32::Messages::Destroy)
                return (Win32::PostQuitMessage(0), 0);
            return Win32::DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }
    };

    template<typename TImplementation>
    class Control
    {
    protected:
        static auto SubclassProc(
            Win32::HWND hwnd,
            unsigned uMsg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam,
            Win32::UINT_PTR,
            Win32::DWORD_PTR dwRefData
        ) -> Win32::LRESULT
        {
            if (auto pThis = reinterpret_cast<TImplementation*>(dwRefData); pThis)
                if (auto result = pThis->HandleMessage(hwnd, uMsg, wParam, lParam); result)
                    return *result;
            return Win32::DefSubclassProc(hwnd, uMsg, wParam, lParam);
        }
    };

    class Label : public Control<Label>
    {
    public:
        ~Label()
        {
            if (not m_hwnd)
                return;
            auto tmp = m_hwnd;
            m_hwnd = nullptr;
            Win32::RemoveWindowSubclass(tmp, &Control<Label>::SubclassProc, 0);
            Win32::DestroyWindow(tmp);
        }

        Label(const Label&) = delete;
        auto operator=(const Label&) -> Label & = delete;

        Label(Label&& other) noexcept
            : m_hwnd{ other.m_hwnd }
        {
            other.m_hwnd = nullptr;
            if (not Win32::SetWindowSubclass(m_hwnd, &SubclassProc, 0, reinterpret_cast<Win32::DWORD_PTR>(this)))
                throw std::runtime_error{ "Failed to track control window destruction" };
        }

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
            Create(controlId, text, parent, x, y, width, height, additionalStyles);
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

        [[nodiscard]]
        auto GetHwnd() const noexcept -> Win32::HWND
        {
            return m_hwnd;
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

    protected:
        void Create(
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
            auto hwnd = Win32::CreateWindowExW(
                0,
                L"STATIC",
                text.c_str(),
                Win32::WindowStyles::Child | Win32::WindowStyles::Visible | additionalStyles,
                x, y,
                width, height,
                parent,
                reinterpret_cast<Win32::HMENU>(static_cast<Win32::ULONG_PTR>(controlId)),
                Win32::GetModuleHandleW(nullptr),
                nullptr);
            if (not hwnd)
                throw Error::Win32Error{ Win32::GetLastError(), "CreateWindowExW failed" };
            if (not Win32::SetWindowSubclass(hwnd, &SubclassProc, 0, reinterpret_cast<Win32::DWORD_PTR>(this)))
                throw std::runtime_error{ "Failed to track control window destruction" };

            m_hwnd = hwnd;
        }

        Win32::HWND m_hwnd = nullptr;
    };

    class Layout
    {
    public:
        Layout(Win32::HWND reference, unsigned rows, unsigned columns, auto rowGap, auto columnGap)
            : m_reference(reference), m_rows(rows), m_columns(columns), m_rowGap(rowGap), m_columnGap(columnGap)
        {
            if (not reference)
                throw std::invalid_argument{ "reference window handle is null" };
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

        auto Apply(this auto&& self) -> decltype(self)
        {
            auto referenceRect = Win32::RECT{};
            Win32::GetClientRect(self.m_reference, &referenceRect);

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
        Win32::HWND m_reference = nullptr;
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

    auto NextControlId() -> unsigned
    {
        static auto nextId = 1u;
        return nextId++;
    }

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