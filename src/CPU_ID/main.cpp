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
}

namespace UI
{
    class Window
    {
    public:
		~Window()
		{
            if (m_hwnd)
            {
				Win32::SetWindowLongPtrW(m_hwnd, Win32::Gwlp::UserData, 0);
                Win32::DestroyWindow(m_hwnd);
				m_hwnd = nullptr;
            }
		}

		Window(Win32::HWND hwnd)
			: m_hwnd(hwnd)
		{}

		auto HandleMessage(Win32::HWND hwnd, unsigned uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> std::optional<Win32::LRESULT>
		{
			return std::nullopt;
		}

		void release()
		{
			m_hwnd = nullptr;
		}
    private:
		Win32::HWND m_hwnd = nullptr;
    };

	template<Strings::FixedString ClassName, typename TWindow>
    struct WindowClass
    {
		WindowClass() 
        { 
            Register(); 
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

		auto CreateWindow() const -> std::unique_ptr<TWindow>
		{
			auto hInstance = Win32::GetModuleHandleW(nullptr);
			if (not hInstance)
				throw Error::Win32Error{ Win32::GetLastError(), "GetModuleHandleW failed" };

			auto hwnd = Win32::CreateWindowExW(
				0,
				ClassName.ToView().data(),
				L"My Window",
				Win32::WindowStyles::OverlappedWindow | Win32::WindowStyles::Visible,
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
}

auto wWinMain(Win32::HINSTANCE, Win32::HINSTANCE, Win32::LPWSTR, int) -> int
{
	using WindowClass = UI::WindowClass<L"MyWindowClass", UI::Window>;

	auto windowClass = WindowClass{};
	auto window = windowClass.CreateWindow();

	auto msg = Win32::MSG{};
	while (Win32::GetMessageW(&msg, nullptr, 0, 0))
	{
		Win32::TranslateMessage(&msg);
		Win32::DispatchMessageW(&msg);
	}
	return static_cast<int>(msg.wParam);
}
