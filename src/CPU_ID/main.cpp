#pragma comment(lib, "Comctl32.lib")

import std;
import CpuId;

class App
{
public:
    using MainWindowClass = UI::WindowClass<L"CpuViewer", UI::TopLevelWindow, true>;
    using PanelClass = UI::WindowClass<L"CpuViewerPanel", UI::Panel, false>;

    App()
    {
        Initialize();
    }

	auto Run() -> int
	{
        auto msg = Win32::MSG{};
        while (Win32::GetMessageW(&msg, nullptr, 0, 0))
        {
            Win32::TranslateMessage(&msg);
            Win32::DispatchMessageW(&msg);
        }
        return static_cast<int>(msg.wParam);
	}

private:
    void ApplyLayout()
    {
        auto layout = UI::GridLayout{
            m_panel->GetHwnd(),
            static_cast<unsigned>(m_labels.size()),
            2,
            0,
            20
        };
        for (unsigned i = 0; i < m_labels.size(); ++i)
        {
            layout
                .AddWindow(m_labels[i].Caption.GetHwnd(), i, 0)
                .AddWindow(m_labels[i].Value.GetHwnd(), i, 1);
        }
        layout.Apply();

		auto anchoredLayout = UI::AnchorLayout{ m_panel->GetHwnd() };
        anchoredLayout.Add(
            UI::AnchorLayout::Entry{
                m_closeButton.GetHwnd(),
                UI::AnchorLayout::Anchor::BottomRight,
                10,
                10
            })
            .Apply();
    }

	void Initialize()
	{
		ApplyLayout();
        m_window->SetFontOnChildren(UI::SystemFont);
    }

    //
	// Private members
    MainWindowClass m_mainWindowClass{};
    std::unique_ptr<UI::TopLevelWindow> m_window =
        m_mainWindowClass.CreateWindow(
            L"CPU Viewer", 
            (Win32::WindowStyles::OverlappedWindow & ~Win32::WindowStyles::ThickFrame & ~Win32::WindowStyles::MaximizeBox) | Win32::WindowStyles::Visible,
            0, 
            0, 0, 
            800, 850, 
            nullptr);

    PanelClass m_panelClass = PanelClass{};
    std::unique_ptr<UI::Panel> m_panel =
        [this] -> std::unique_ptr<UI::Panel>
        {
            // Create the panel that will hold the labels
            auto clientDimensions = m_window->GetClientDimensions();
            return m_panelClass.CreateWindow(
                L"",
                Win32::WindowStyles::Child | Win32::WindowStyles::Visible | Win32::WindowStyles::ClipChildren,
                0,
                0, 0,
                clientDimensions.Width, clientDimensions.Height,
                m_window->GetHwnd());
        }();

	UI::Button m_closeButton = UI::Button{
		UI::NextControlId(),
		L"Close",
        m_panel->GetHwnd(),
		0, 0,
		100, 30,
		Win32::WindowStyles::Visible | Win32::WindowStyles::Child
	};

    // Create the labels for each instruction set feature
    std::array<UI::CaptionedLabel, 54> m_labels =
        std::array{
            UI::CaptionedLabel{ L"Manufacturer:", Strings::Convert(InstructionSet::Vendor()), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"Brand:", Strings::Convert(InstructionSet::Brand()), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"SSE3:", InstructionSet::SSE3(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"PCLMULQDQ:", InstructionSet::PCLMULQDQ(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"MONITOR:", InstructionSet::MONITOR(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"SSSE3:", InstructionSet::SSSE3(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"FMA:", InstructionSet::FMA(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"CMPXCHG16B:", InstructionSet::CMPXCHG16B(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"SSE41:", InstructionSet::SSE41(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"SSE42:", InstructionSet::SSE42(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"MOVBE:", InstructionSet::MOVBE(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"POPCNT:", InstructionSet::POPCNT(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"AES:", InstructionSet::AES(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"XSAVE:", InstructionSet::XSAVE(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"OSXSAVE:", InstructionSet::OSXSAVE(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"AVX:", InstructionSet::AVX(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"F16C:", InstructionSet::F16C(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"RDRAND:", InstructionSet::RDRAND(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"MSR:", InstructionSet::MSR(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"CX8:", InstructionSet::CX8(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"SEP:", InstructionSet::SEP(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"CMOV:", InstructionSet::CMOV(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"CLFSH:", InstructionSet::CLFSH(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"MMX:", InstructionSet::MMX(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"FXSR:", InstructionSet::FXSR(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"SSE:", InstructionSet::SSE(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"SSE2:", InstructionSet::SSE2(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"FSGSBASE:", InstructionSet::FSGSBASE(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"BMI1:", InstructionSet::BMI1(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"HLE:", InstructionSet::HLE(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"AVX2:", InstructionSet::AVX2(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"BMI2:", InstructionSet::BMI2(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"ERMS:", InstructionSet::ERMS(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"INVPCID:", InstructionSet::INVPCID(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"RTM:", InstructionSet::RTM(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"AVX512F:", InstructionSet::AVX512F(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"RDSEED:", InstructionSet::RDSEED(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"ADX:", InstructionSet::ADX(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"AVX512PF:", InstructionSet::AVX512PF(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"AVX512ER:", InstructionSet::AVX512ER(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"AVX512CD:", InstructionSet::AVX512CD(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"SHA:", InstructionSet::SHA(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"PREFETCHWT1:", InstructionSet::PREFETCHWT1(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"LAHF:", InstructionSet::LAHF(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"LZCNT:", InstructionSet::LZCNT(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"ABM:", InstructionSet::ABM(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"SSE4a:", InstructionSet::SSE4a(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"XOP:", InstructionSet::XOP(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"TBM:", InstructionSet::TBM(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"SYSCALL:", InstructionSet::SYSCALL(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"MMXEXT:", InstructionSet::MMXEXT(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"RDTSCP:", InstructionSet::RDTSCP(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"3DNOWEXT:", InstructionSet::_3DNOWEXT(), m_panel->GetHwnd() },
            UI::CaptionedLabel{ L"3DNOW:", InstructionSet::_3DNOW(), m_panel->GetHwnd() },
        };
};

auto wWinMain(Win32::HINSTANCE, Win32::HINSTANCE, Win32::LPWSTR, int) -> int
{
	return App{}.Run();
}
