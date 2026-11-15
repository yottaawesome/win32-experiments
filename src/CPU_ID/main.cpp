#pragma comment(lib, "Comctl32.lib")

import std;
import Win32;
import Intrinsics;

namespace InstructionSet
{
    struct InstructionSet_Internal
    {
        InstructionSet_Internal()
        {
            //int cpuInfo[4] = {-1};
            auto cpui = std::array<int, 4>{};

            // Calling __cpuid with 0x0 as the function_id argument
            // gets the number of the highest valid function ID.
            __cpuid(cpui.data(), 0);
            nIds_ = cpui[0];

            for (int i = 0; i <= nIds_; ++i)
            {
                __cpuidex(cpui.data(), i, 0);
                data_.push_back(cpui);
            }

            // Capture vendor string
            auto vendor = std::array<char, 0x20>{};
            *reinterpret_cast<int*>(vendor.data()) = data_[0][1];
            *reinterpret_cast<int*>(vendor.data() + 4) = data_[0][3];
            *reinterpret_cast<int*>(vendor.data() + 8) = data_[0][2];
            vendor_ = vendor.data();
            if (vendor_ == "GenuineIntel")
            {
                isIntel_ = true;
            }
            else if (vendor_ == "AuthenticAMD")
            {
                isAMD_ = true;
            }

            // load bitset with flags for function 0x00000001
            if (nIds_ >= 1)
            {
                f_1_ECX_ = data_[1][2];
                f_1_EDX_ = data_[1][3];
            }

            // load bitset with flags for function 0x00000007
            if (nIds_ >= 7)
            {
                f_7_EBX_ = data_[7][1];
                f_7_ECX_ = data_[7][2];
            }

            // Calling __cpuid with 0x80000000 as the function_id argument
            // gets the number of the highest valid extended ID.
            __cpuid(cpui.data(), 0x80000000);
            nExIds_ = cpui[0];

            auto brand = std::array<char, 0x40>{};

            for (int i = 0x80000000; i <= nExIds_; ++i)
            {
                __cpuidex(cpui.data(), i, 0);
                extdata_.push_back(cpui);
            }

            // load bitset with flags for function 0x80000001
            if (nExIds_ >= 0x80000001)
            {
                f_81_ECX_ = extdata_[1][2];
                f_81_EDX_ = extdata_[1][3];
            }

            // Interpret CPU brand string if reported
            if (nExIds_ >= 0x80000004)
            {
                std::memcpy(brand.data(), extdata_[2].data(), sizeof(cpui));
                std::memcpy(brand.data() + 16, extdata_[3].data(), sizeof(cpui));
                std::memcpy(brand.data() + 32, extdata_[4].data(), sizeof(cpui));
                brand_ = brand.data();
            }
        };

        int nIds_ = 0;
        int nExIds_ = 0;
        std::string vendor_;
        std::string brand_;
        bool isIntel_ = false;
        bool isAMD_ = false;
        std::bitset<32> f_1_ECX_{ 0 };
        std::bitset<32> f_1_EDX_{ 0 };
        std::bitset<32> f_7_EBX_{ 0 };
        std::bitset<32> f_7_ECX_{ 0 };
        std::bitset<32> f_81_ECX_{ 0 };
        std::bitset<32> f_81_EDX_{ 0 };
        std::vector<std::array<int, 4>> data_{};
        std::vector<std::array<int, 4>> extdata_{};
    };

    static const auto CPU_Rep = InstructionSet_Internal{};

    // getters
    auto Vendor() 
        noexcept -> std::string { return CPU_Rep.vendor_; }
    auto Brand() 
        noexcept -> std::string { return CPU_Rep.brand_; }

    auto SSE3() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[0]; }
    auto PCLMULQDQ() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[1]; }
    auto MONITOR() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[3]; }
    auto SSSE3() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[9]; }
    auto FMA() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[12]; }
    auto CMPXCHG16B() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[13]; }
    auto SSE41() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[19]; }
    auto SSE42() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[20]; }
    auto MOVBE() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[22]; }
    auto POPCNT() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[23]; }
    auto AES() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[25]; }
    auto XSAVE() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[26]; }
    auto OSXSAVE() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[27]; }
    auto AVX() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[28]; }
    auto F16C() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[29]; }
    auto RDRAND() 
        noexcept -> bool { return CPU_Rep.f_1_ECX_[30]; }

    auto MSR() 
        noexcept -> bool { return CPU_Rep.f_1_EDX_[5]; }
    auto CX8() 
        noexcept -> bool { return CPU_Rep.f_1_EDX_[8]; }
    auto SEP() 
        noexcept -> bool { return CPU_Rep.f_1_EDX_[11]; }
    auto CMOV() 
        noexcept -> bool { return CPU_Rep.f_1_EDX_[15]; }
    auto CLFSH() 
        noexcept -> bool { return CPU_Rep.f_1_EDX_[19]; }
    auto MMX() 
        noexcept -> bool { return CPU_Rep.f_1_EDX_[23]; }
    auto FXSR() 
        noexcept -> bool { return CPU_Rep.f_1_EDX_[24]; }
    auto SSE() 
        noexcept -> bool { return CPU_Rep.f_1_EDX_[25]; }
    auto SSE2() 
        noexcept -> bool { return CPU_Rep.f_1_EDX_[26]; }

    auto FSGSBASE() 
        noexcept -> bool { return CPU_Rep.f_7_EBX_[0]; }
    auto BMI1() 
        noexcept -> bool { return CPU_Rep.f_7_EBX_[3]; }
    auto HLE() 
        noexcept -> bool { return CPU_Rep.isIntel_ && CPU_Rep.f_7_EBX_[4]; }
    auto AVX2() 
        noexcept -> bool { return CPU_Rep.f_7_EBX_[5]; }
    auto BMI2() 
        noexcept -> bool { return CPU_Rep.f_7_EBX_[8]; }
    auto ERMS() 
        noexcept -> bool { return CPU_Rep.f_7_EBX_[9]; }
    auto INVPCID() 
        noexcept -> bool { return CPU_Rep.f_7_EBX_[10]; }
    auto RTM() 
        noexcept -> bool { return CPU_Rep.isIntel_ && CPU_Rep.f_7_EBX_[11]; }
    auto AVX512F() 
        noexcept -> bool { return CPU_Rep.f_7_EBX_[16]; }
    auto RDSEED() 
        noexcept -> bool { return CPU_Rep.f_7_EBX_[18]; }
    auto ADX() 
        noexcept -> bool { return CPU_Rep.f_7_EBX_[19]; }
    auto AVX512PF() 
        noexcept -> bool { return CPU_Rep.f_7_EBX_[26]; }
    auto AVX512ER() 
        noexcept -> bool { return CPU_Rep.f_7_EBX_[27]; }
    auto AVX512CD() 
        noexcept -> bool { return CPU_Rep.f_7_EBX_[28]; }
    auto SHA() 
        noexcept -> bool { return CPU_Rep.f_7_EBX_[29]; }

    auto PREFETCHWT1() 
        noexcept -> bool { return CPU_Rep.f_7_ECX_[0]; }

    auto LAHF() 
        noexcept -> bool { return CPU_Rep.f_81_ECX_[0]; }
    auto LZCNT() 
        noexcept -> bool { return CPU_Rep.isIntel_ && CPU_Rep.f_81_ECX_[5]; }
    auto ABM() 
        noexcept -> bool { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[5]; }
    auto SSE4a() 
        noexcept -> bool { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[6]; }
    auto XOP() 
        noexcept -> bool { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[11]; }
    auto TBM() 
        noexcept -> bool { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[21]; }

    auto SYSCALL() 
        noexcept -> bool { return CPU_Rep.isIntel_ && CPU_Rep.f_81_EDX_[11]; }
    auto MMXEXT() 
        noexcept -> bool { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[22]; }
    auto RDTSCP() 
        noexcept -> bool { return CPU_Rep.isIntel_ && CPU_Rep.f_81_EDX_[27]; }
    auto _3DNOWEXT() 
        noexcept -> bool { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[30]; }
    auto _3DNOW() 
        noexcept -> bool { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[31]; }
};

namespace Error
{
	auto TranslateWin32Error(Win32::DWORD errorCode) -> std::string
	{
		auto messageBuffer = Win32::LPWSTR{};
		auto size = Win32::FormatMessageW(
			Win32::FormatMessageFlags::AllocateBuffer | Win32::FormatMessageFlags::FromSystem | Win32::FormatMessageFlags::IgnoreInserts,
			nullptr,
			errorCode,
			0,
			reinterpret_cast<Win32::LPWSTR>(&messageBuffer),
			0,
			nullptr);
		if (size == 0)
			return std::format("Unknown error code: {}", errorCode);
		auto message = std::wstring(messageBuffer, size);
		Win32::LocalFree(messageBuffer);
		while (not message.empty() and (message.back() == L'\n' or message.back() == L'\r'))
			message.pop_back();
		return std::string(message.begin(), message.end());
	}

    class Win32Error : public std::runtime_error
    {
    public:
        Win32Error(Win32::DWORD errorCode)
            : std::runtime_error(std::format("{}", TranslateWin32Error(errorCode)))
            , m_errorCode(errorCode)
        {}

        Win32Error(Win32::DWORD errorCode, const std::string& message)
            : std::runtime_error(std::format("{}: {}", message, TranslateWin32Error(errorCode)))
            , m_errorCode(errorCode)
        {}

        [[nodiscard]]
        constexpr auto ErrorCode() const noexcept -> Win32::DWORD
        {
            return m_errorCode;
        }
    private:
        Win32::DWORD m_errorCode = 0;
    };
}

namespace Strings
{
    template<size_t N, typename TChar>
	struct FixedString
	{
		using TView = std::basic_string_view<TChar>;
		using TString = std::basic_string<TChar>;

        TChar value[N];
        constexpr FixedString(const TChar(&str)[N])
		{
			std::copy_n(str, N, value);
		}
		[[nodiscard]]
		constexpr auto ToView() const noexcept -> TView
		{
			return TView{ value, N - 1 };
		}
        [[nodiscard]]
        constexpr auto ToString() const noexcept -> TString
		{
			return TString{ value, N - 1 };
		}
	};

	template<size_t N>
	FixedString(const char(&)[N]) -> FixedString<N, char>;
    template<size_t N>
    FixedString(const wchar_t(&)[N]) -> FixedString<N, wchar_t>;

    auto Convert(const std::string& str) -> std::wstring
    {
        if (str.empty())
            return {};
        auto sizeNeeded = Win32::MultiByteToWideChar(
            Win32::CodePage::Utf8,
            0,
            str.data(),
            static_cast<int>(str.size()),
            nullptr,
            0
        );
        if (sizeNeeded <= 0)
            throw Error::Win32Error{ Win32::GetLastError(), "MultiByteToWideChar() failed [1]" };

        auto wstr = std::wstring(sizeNeeded, '\0');
        auto result = Win32::MultiByteToWideChar(
            Win32::CodePage::Utf8,
            0,
            str.data(),
            static_cast<int>(str.size()),
            wstr.data(),
            sizeNeeded
        );
        if (result <= 0)
            throw Error::Win32Error{ Win32::GetLastError(), "MultiByteToWideChar() failed [2]" };

        return wstr;
    }
}

namespace RAII
{
    template<auto VDeleteFn>
    struct Deleter
    {
        static constexpr void operator()(auto handle) noexcept
        {
            VDeleteFn(handle);
        }
    };
    template<typename T, auto VDeleter>
    using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<T>, Deleter<VDeleter>>;
    using FontUniquePtr = IndirectUniquePtr<Win32::HFONT, Win32::DeleteObject>;
}

namespace UI
{
    template<std::invocable CreateFn>
    struct Font
    {
        CreateFn Fn;

        auto Get() const -> Win32::HFONT
        {
            static auto font = RAII::FontUniquePtr{ Fn() };
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

    class Window
    {
    public:
		~Window()
		{
            if (m_hwnd)
            {
                auto tmp = m_hwnd;
				release();
                Win32::DestroyWindow(tmp);
            }
		}

		Window(Win32::HWND hwnd)
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

        auto CreateWindow(const std::wstring& title, Win32::DWORD styles, Win32::DWORD exStyles) const -> std::unique_ptr<TWindow>
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
                800, 600,
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
					pThis = new TWindow{hwnd};
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

		Label(Label&& other)
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
				throw std::invalid_argument("parent window handle is null");
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
				throw std::runtime_error("Label window handle is null");
			Win32::SetWindowTextW(m_hwnd, text.c_str());
		}

        auto SetText(bool value)
        {
            if (not m_hwnd)
                throw std::runtime_error("Label window handle is null");
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
		Layout(Win32::HWND reference, unsigned rows, unsigned columns)
			: m_reference(reference), m_rows(rows), m_columns(columns)
		{
			if (not reference)
				throw std::invalid_argument("reference window handle is null");
			if (rows == 0 or columns == 0)
				throw std::invalid_argument("rows and columns must be greater than zero");
		}

        auto AddWindow(this auto&& self, Win32::HWND hwnd, unsigned row, unsigned column) -> decltype(self)
        {
            if (not hwnd)
				throw std::invalid_argument("window handle is null");
			self.m_toSize.push_back({hwnd, row, column});
            return self;
        }

        auto Apply(this auto&& self) -> decltype(self)
        {
			auto referenceRect = Win32::RECT{};
            Win32::GetClientRect(self.m_reference, &referenceRect);

            auto rowHeight = (referenceRect.bottom - referenceRect.top) / self.m_rows;
            auto colWidth = (referenceRect.right - referenceRect.left) / self.m_columns;

			for (auto& wnd : self.m_toSize)
			{
				auto x = colWidth * wnd.column;
				auto y = rowHeight * wnd.row;
				Win32::SetWindowPos(wnd.hwnd, nullptr, x, y, colWidth, rowHeight, Win32::SetWindowPosFlags::NoZOrder);
			}
            return self;
        }

    private:
		unsigned m_rows = 1;
		unsigned m_columns = 1;
		Win32::HWND m_reference = nullptr;
        struct Position
		{
			Win32::HWND hwnd = nullptr;
            unsigned row = 0;
			unsigned column = 0;
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
        { }

        CaptionedLabel(const std::wstring& caption, bool value, Win32::HWND parent)
            : Caption{ UI::NextControlId(), caption, parent, 0, 0, 100, 30, Win32::SS::Right }
            , Value{ UI::NextControlId(), value ? L"true" : L"false", parent, 0, 0, 100, 30, Win32::SS::Left }
        {}
	};
}

auto wWinMain(Win32::HINSTANCE, Win32::HINSTANCE, Win32::LPWSTR, int) -> int
{
	using WindowClass = UI::WindowClass<L"CpuViewer", UI::Window>;

	auto windowClass = WindowClass{};
	auto window = windowClass.CreateWindow(L"CPU Viewer", Win32::WindowStyles::OverlappedWindow | Win32::WindowStyles::Visible, 0);

	auto labels = std::array{
        UI::CaptionedLabel{ L"Manufacturer: ", Strings::Convert(InstructionSet::Vendor()), window->GetHwnd() },
        UI::CaptionedLabel{ L"Brand: ", Strings::Convert(InstructionSet::Brand()), window->GetHwnd() },
        UI::CaptionedLabel{ L"SSE3: ", InstructionSet::SSE(), window->GetHwnd() },
        UI::CaptionedLabel{ L"PCLMULQDQ: ", InstructionSet::PCLMULQDQ(), window->GetHwnd() },
        UI::CaptionedLabel{ L"MONITOR: ", InstructionSet::MONITOR(), window->GetHwnd() },
        UI::CaptionedLabel{ L"SSSE3: ", InstructionSet::SSSE3(), window->GetHwnd() },
        UI::CaptionedLabel{ L"FMA: ", InstructionSet::FMA(), window->GetHwnd() },
	};

    auto layout = UI::Layout{ window->GetHwnd(), static_cast<unsigned>(std::max(labels.size(), 20ull)), 2};
	for (unsigned i = 0; i < labels.size(); ++i)
	{
		layout.AddWindow(labels[i].Caption.GetHwnd(), i, 0);
		layout.AddWindow(labels[i].Value.GetHwnd(), i, 1);
	}
    layout.Apply();

    window->SetFontOnChildren(UI::SystemFont);

	auto msg = Win32::MSG{};
	while (Win32::GetMessageW(&msg, nullptr, 0, 0))
	{
		Win32::TranslateMessage(&msg);
		Win32::DispatchMessageW(&msg);
	}
	return static_cast<int>(msg.wParam);
}
