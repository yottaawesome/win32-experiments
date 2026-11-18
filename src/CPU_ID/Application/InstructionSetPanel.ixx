export module CpuId:Application.InstructionSetPanel;
import std;
import Win32;
import :UI;
import :Error;
import :InstructionSet;
import :stdx;
import :Application.Common;

export namespace Application
{
    struct CpuInstructionSetPanel
    {
        //
		// Special member functions
    public:
        CpuInstructionSetPanel(
            UI::TopLevelWindow& mainWindow,
            UI::PanelClass& panelClass,
            std::move_only_function<auto()->void> onClick
        ) : MainWindow{ mainWindow }
            , PanelClass{ panelClass }
        {
            MoreCpuInfoBtn.OnClick = std::move(onClick);
            ApplyLayout();
        }

        //
		// Public methods
#pragma region Public methods
        void ApplyLayout()
        {
            auto layout = UI::GridLayout{
                static_cast<unsigned>(Labels.size()),
                2,
                0,
                20
            };
            for (unsigned i = 0; i < Labels.size(); ++i)
            {
                layout
                    .AddWindow(Labels[i].Caption.GetHwnd(), i, 0)
                    .AddWindow(Labels[i].Value.GetHwnd(), i, 1);
            }
            // We reduce the bottom of the panel by 40 pixels to make room for the "More CPU Info" button.
            // Otherwise, the GridLayout will stretch the rows to fill the width and height of the panel, 
            // which would cause the button to be drawn over by the labels and it seemingly disappearing.
            auto dimensions = Panel->GetClientRect();
            dimensions.bottom -= 40;
            layout.Apply(dimensions);

            UI::AnchorLayout{ Panel->GetHwnd() }
                .Add(
                    UI::AnchorLayout::Entry{
                        MoreCpuInfoBtn.GetHwnd(),
                        UI::AnchorLayout::Anchor::BottomRight,
                        10,
                        10
                    })
                .Apply();
        }

        auto Show()
        {
            Panel->Show();
        }

        auto Hide()
        {
            Panel->Hide();
        }
#pragma endregion

        //
		// Public fields
#pragma region UI fields
        UI::TopLevelWindow& MainWindow;
        UI::PanelClass& PanelClass;

        std::unique_ptr<UI::Panel> Panel =
            [this] -> std::unique_ptr<UI::Panel>
            {
                // Create the panel that will hold the labels
                auto clientDimensions = MainWindow.GetClientDimensions();
                return PanelClass.CreateWindow(
                    L"",
                    0,
                    0,
                    0, 0,
                    clientDimensions.Width, clientDimensions.Height,
                    MainWindow.GetHwnd());
            }();

        UI::Button MoreCpuInfoBtn = UI::Button{
            UI::ButtonArgs{
                .Control{
                    .ControlId = UI::NextControlId(),
                    .Text = L"Show current CPU performance",
                    .Parent = Panel->GetHwnd(),
                    .Width = 250,
                    .Height = 30
                }
            }
        };

        // Create the labels for each instruction set feature
        std::array<UI::CaptionedLabel, 54> Labels =
            std::array{
                UI::CaptionedLabel{ L"Manufacturer:", Strings::Convert(InstructionSet::Vendor()), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"Brand:", Strings::Convert(InstructionSet::Brand()), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"SSE3:", InstructionSet::SSE3(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"PCLMULQDQ:", InstructionSet::PCLMULQDQ(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"MONITOR:", InstructionSet::MONITOR(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"SSSE3:", InstructionSet::SSSE3(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"FMA:", InstructionSet::FMA(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"CMPXCHG16B:", InstructionSet::CMPXCHG16B(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"SSE41:", InstructionSet::SSE41(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"SSE42:", InstructionSet::SSE42(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"MOVBE:", InstructionSet::MOVBE(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"POPCNT:", InstructionSet::POPCNT(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"AES:", InstructionSet::AES(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"XSAVE:", InstructionSet::XSAVE(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"OSXSAVE:", InstructionSet::OSXSAVE(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"AVX:", InstructionSet::AVX(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"F16C:", InstructionSet::F16C(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"RDRAND:", InstructionSet::RDRAND(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"MSR:", InstructionSet::MSR(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"CX8:", InstructionSet::CX8(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"SEP:", InstructionSet::SEP(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"CMOV:", InstructionSet::CMOV(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"CLFSH:", InstructionSet::CLFSH(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"MMX:", InstructionSet::MMX(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"FXSR:", InstructionSet::FXSR(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"SSE:", InstructionSet::SSE(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"SSE2:", InstructionSet::SSE2(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"FSGSBASE:", InstructionSet::FSGSBASE(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"BMI1:", InstructionSet::BMI1(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"HLE:", InstructionSet::HLE(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"AVX2:", InstructionSet::AVX2(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"BMI2:", InstructionSet::BMI2(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"ERMS:", InstructionSet::ERMS(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"INVPCID:", InstructionSet::INVPCID(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"RTM:", InstructionSet::RTM(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"AVX512F:", InstructionSet::AVX512F(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"RDSEED:", InstructionSet::RDSEED(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"ADX:", InstructionSet::ADX(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"AVX512PF:", InstructionSet::AVX512PF(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"AVX512ER:", InstructionSet::AVX512ER(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"AVX512CD:", InstructionSet::AVX512CD(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"SHA:", InstructionSet::SHA(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"PREFETCHWT1:", InstructionSet::PREFETCHWT1(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"LAHF:", InstructionSet::LAHF(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"LZCNT:", InstructionSet::LZCNT(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"ABM:", InstructionSet::ABM(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"SSE4a:", InstructionSet::SSE4a(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"XOP:", InstructionSet::XOP(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"TBM:", InstructionSet::TBM(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"SYSCALL:", InstructionSet::SYSCALL(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"MMXEXT:", InstructionSet::MMXEXT(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"RDTSCP:", InstructionSet::RDTSCP(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"3DNOWEXT:", InstructionSet::_3DNOWEXT(), Panel->GetHwnd() },
                UI::CaptionedLabel{ L"3DNOW:", InstructionSet::_3DNOW(), Panel->GetHwnd() },
        };
#pragma endregion
    };
}