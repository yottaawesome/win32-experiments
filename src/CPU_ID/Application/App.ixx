export module CpuId:Application.App;
import std;
import Win32;
import :UI;
import :stdx;
import :Application.Common;
import :Application.InstructionSetPanel;
import :Application.CpuPerformancePanel;

export namespace Application
{
    class App
    {
        //
		// Special member functions
    public:
        App()
        {
            Initialize();
        }

        //
		// Public methods
    public:
        auto Run() -> int
        {
            auto msg = Win32::MSG{};
            while (true)
            {
                auto result = Win32::GetMessageW(&msg, nullptr, 0, 0);
                if (result == -1)
                    throw Error::Win32Error{ Win32::GetLastError(), "GetMessageW failed" };
                if (result == 0)
                    return static_cast<int>(msg.wParam);
                Win32::TranslateMessage(&msg);
                Win32::DispatchMessageW(&msg);
            }
        }

        //
		// Private methods
    private:
        void Initialize()
        {
            InitializeFirstPanel();
        }

        template<typename TPanelTo>
        void SwapPanels(std::move_only_function<auto()->void> swapFunction)
        {
            // This is done to minimize flickering when switching panels.
            // First, we create the panel to swap to and keep it hidden.
            auto toShow = std::make_unique<TPanelTo>(*m_window, m_panelClass, std::move(swapFunction));
            m_window->SetFontOnChildren(UI::SystemFont);
            // Then, we hide the current panel and show the new one.
            m_panels.Visit([](std::monostate) {}, [](auto&& panel) { panel->Hide(); });
            toShow->Show();
            // Finally, we swap the new panel into the variant so that it is now the current panel.
            m_panels.Emplace<std::unique_ptr<TPanelTo>>(std::move(toShow));
        }

        void InitializeFirstPanel()
        {
            SwapPanels<CpuInstructionSetPanel>([this] { InitializeSecondPanel(); });
        }

        void InitializeSecondPanel()
        {
            SwapPanels<CpuPerformancePanel>([this] { InitializeFirstPanel(); });
        }

        //
        // Private members
    private:
        MainWindowClass m_mainWindowClass{};
        UI::PanelClass m_panelClass{};
        std::unique_ptr<UI::TopLevelWindow> m_window =
            m_mainWindowClass.CreateWindow(
                L"CPU Viewer",
				0, 0,       // no extra styles
				0, 0,       // x, y
				800, 900,   // width, height
                nullptr);

        // Note that we use pointers in the variant. This isn't strictly required,
        // but it's done like this to minimize flickering between panel swaps,
        // which requires both panels to be alive at the same time for a brief 
        // moment, with the original panel only being destroyed after it is hidden 
        // and the new panel is shown.
        stdx::Variant<
            std::monostate,
            std::unique_ptr<CpuInstructionSetPanel>,
            std::unique_ptr<CpuPerformancePanel>
        > m_panels;
    };
}