export module CpuId:Application.CpuPerformancePanel;
import std;
import Win32;
import :UI;

export namespace Application
{
    struct CpuPerformancePanel
    {
        //
        // Special member functions
        CpuPerformancePanel(
            UI::TopLevelWindow& mainWindow,
            UI::PanelClass& panelClass,
            std::move_only_function<auto()->void> onClick
        ) : MainWindow{ mainWindow }
            , PanelClass{ panelClass }
        {
            ShowBasicCpuInfoBtn.OnClick = std::move(onClick);
            ApplyLayout();
        }

        //
        // Public methods
#pragma region Public methods
        void ApplyLayout()
        {
            UI::AnchorLayout{ Panel->GetHwnd() }
                .Add(
                    UI::AnchorLayout::Entry{
                        ShowBasicCpuInfoBtn.GetHwnd(),
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
                    Win32::WindowStyles::Child | Win32::WindowStyles::ClipChildren,
                    0,
                    0, 0,
                    clientDimensions.Width, clientDimensions.Height,
                    MainWindow.GetHwnd());
            }();

        UI::Button ShowBasicCpuInfoBtn = UI::Button{
            UI::ButtonArgs{
                .Control{
                    .ControlId = UI::NextControlId(),
                    .Text = L"Show basic CPU info",
                    .Parent = Panel->GetHwnd(),
                    .Width = 250,
                    .Height = 30,
                }
            }
        };
#pragma endregion
    };
}