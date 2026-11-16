#pragma comment(lib, "Comctl32.lib")

import std;
import CpuId;

auto wWinMain(Win32::HINSTANCE, Win32::HINSTANCE, Win32::LPWSTR, int) -> int
{
	using WindowClass = UI::WindowClass<L"CpuViewer", UI::TopLevelWindow>;

	auto windowClass = WindowClass{};
	constexpr auto windowStyle = 
        (Win32::WindowStyles::OverlappedWindow & ~Win32::WindowStyles::ThickFrame & ~Win32::WindowStyles::MaximizeBox) | Win32::WindowStyles::Visible;
	auto window = windowClass.CreateWindow(L"CPU Viewer", windowStyle, 0, 800, 850);

	auto labels = std::array{
        UI::CaptionedLabel{ L"Manufacturer:", Strings::Convert(InstructionSet::Vendor()), window->GetHwnd() },
        UI::CaptionedLabel{ L"Brand:", Strings::Convert(InstructionSet::Brand()), window->GetHwnd() },
        UI::CaptionedLabel{ L"SSE3:", InstructionSet::SSE3(), window->GetHwnd() },
        UI::CaptionedLabel{ L"PCLMULQDQ:", InstructionSet::PCLMULQDQ(), window->GetHwnd() },
        UI::CaptionedLabel{ L"MONITOR:", InstructionSet::MONITOR(), window->GetHwnd() },
        UI::CaptionedLabel{ L"SSSE3:", InstructionSet::SSSE3(), window->GetHwnd() },
        UI::CaptionedLabel{ L"FMA:", InstructionSet::FMA(), window->GetHwnd() },
        UI::CaptionedLabel{ L"CMPXCHG16B:", InstructionSet::CMPXCHG16B(), window->GetHwnd() },
        UI::CaptionedLabel{ L"SSE41:", InstructionSet::SSE41(), window->GetHwnd() },
        UI::CaptionedLabel{ L"SSE42:", InstructionSet::SSE42(), window->GetHwnd() },
        UI::CaptionedLabel{ L"MOVBE:", InstructionSet::MOVBE(), window->GetHwnd() },
        UI::CaptionedLabel{ L"POPCNT:", InstructionSet::POPCNT(), window->GetHwnd() },
        UI::CaptionedLabel{ L"AES:", InstructionSet::AES(), window->GetHwnd() },
        UI::CaptionedLabel{ L"XSAVE:", InstructionSet::XSAVE(), window->GetHwnd() },
        UI::CaptionedLabel{ L"OSXSAVE:", InstructionSet::OSXSAVE(), window->GetHwnd() },
        UI::CaptionedLabel{ L"AVX:", InstructionSet::AVX(), window->GetHwnd() },
        UI::CaptionedLabel{ L"F16C:", InstructionSet::F16C(), window->GetHwnd() },
        UI::CaptionedLabel{ L"RDRAND:", InstructionSet::RDRAND(), window->GetHwnd() },
        UI::CaptionedLabel{ L"MSR:", InstructionSet::MSR(), window->GetHwnd() },
        UI::CaptionedLabel{ L"CX8:", InstructionSet::CX8(), window->GetHwnd() },
        UI::CaptionedLabel{ L"SEP:", InstructionSet::SEP(), window->GetHwnd() },
        UI::CaptionedLabel{ L"CMOV:", InstructionSet::CMOV(), window->GetHwnd() },
        UI::CaptionedLabel{ L"CLFSH:", InstructionSet::CLFSH(), window->GetHwnd() },
        UI::CaptionedLabel{ L"MMX:", InstructionSet::MMX(), window->GetHwnd() },
        UI::CaptionedLabel{ L"FXSR:", InstructionSet::FXSR(), window->GetHwnd() },
        UI::CaptionedLabel{ L"SSE:", InstructionSet::SSE(), window->GetHwnd() },
        UI::CaptionedLabel{ L"SSE2:", InstructionSet::SSE2(), window->GetHwnd() },
        UI::CaptionedLabel{ L"FSGSBASE:", InstructionSet::FSGSBASE(), window->GetHwnd() },
        UI::CaptionedLabel{ L"BMI1:", InstructionSet::BMI1(), window->GetHwnd() },
        UI::CaptionedLabel{ L"HLE:", InstructionSet::HLE(), window->GetHwnd() },
        UI::CaptionedLabel{ L"AVX2:", InstructionSet::AVX2(), window->GetHwnd() },
        UI::CaptionedLabel{ L"BMI2:", InstructionSet::BMI2(), window->GetHwnd() },
        UI::CaptionedLabel{ L"ERMS:", InstructionSet::ERMS(), window->GetHwnd() },
        UI::CaptionedLabel{ L"INVPCID:", InstructionSet::INVPCID(), window->GetHwnd() },
        UI::CaptionedLabel{ L"RTM:", InstructionSet::RTM(), window->GetHwnd() },
        UI::CaptionedLabel{ L"AVX512F:", InstructionSet::AVX512F(), window->GetHwnd() },
        UI::CaptionedLabel{ L"RDSEED:", InstructionSet::RDSEED(), window->GetHwnd() },
        UI::CaptionedLabel{ L"ADX:", InstructionSet::ADX(), window->GetHwnd() },
        UI::CaptionedLabel{ L"AVX512PF:", InstructionSet::AVX512PF(), window->GetHwnd() },
        UI::CaptionedLabel{ L"AVX512ER:", InstructionSet::AVX512ER(), window->GetHwnd() },
        UI::CaptionedLabel{ L"AVX512CD:", InstructionSet::AVX512CD(), window->GetHwnd() },
        UI::CaptionedLabel{ L"SHA:", InstructionSet::SHA(), window->GetHwnd() },
        UI::CaptionedLabel{ L"PREFETCHWT1:", InstructionSet::PREFETCHWT1(), window->GetHwnd() },
        UI::CaptionedLabel{ L"LAHF:", InstructionSet::LAHF(), window->GetHwnd() },
        UI::CaptionedLabel{ L"LZCNT:", InstructionSet::LZCNT(), window->GetHwnd() },
        UI::CaptionedLabel{ L"ABM:", InstructionSet::ABM(), window->GetHwnd() },
        UI::CaptionedLabel{ L"SSE4a:", InstructionSet::SSE4a(), window->GetHwnd() },
        UI::CaptionedLabel{ L"XOP:", InstructionSet::XOP(), window->GetHwnd() },
        UI::CaptionedLabel{ L"TBM:", InstructionSet::TBM(), window->GetHwnd() },
        UI::CaptionedLabel{ L"SYSCALL:", InstructionSet::SYSCALL(), window->GetHwnd() },
        UI::CaptionedLabel{ L"MMXEXT:", InstructionSet::MMXEXT(), window->GetHwnd() },
        UI::CaptionedLabel{ L"RDTSCP:", InstructionSet::RDTSCP(), window->GetHwnd() },
        UI::CaptionedLabel{ L"3DNOWEXT:", InstructionSet::_3DNOWEXT(), window->GetHwnd() },
        UI::CaptionedLabel{ L"3DNOW:", InstructionSet::_3DNOW(), window->GetHwnd() },
	};

    auto layout = UI::Layout{ window->GetHwnd(), static_cast<unsigned>(labels.size()), 2, 0, 20};
	for (unsigned i = 0; i < labels.size(); ++i)
	{
		layout
            .AddWindow(labels[i].Caption.GetHwnd(), i, 0)
            .AddWindow(labels[i].Value.GetHwnd(), i, 1);
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
