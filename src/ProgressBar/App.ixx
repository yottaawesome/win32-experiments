export module MultiControlSamples:App;
import std;
import Win32;
import :CustomWindow;
import :TopLevelWindow;
import :ProgressBarControl;
import :ButtonControl;
import :StatusBarControl;
import :Panel;
import :Font;

export namespace MultiControlSamples
{
	class App
	{
	public:
		using MainWindow = CustomWindow<TopLevelWindow>;

		App()
		{
			m_progressBar
				.SetRange(0, 100)
				.SetPosition(0)
				.SetIncrement(10);
			m_panel
				.ResizeToParent(0, 0, 150, 0);
			m_statusBar
				.Split(400, -1)
				.SetText(0, L"Ready")
				.SetText(1, L"Not connected");
			m_mainWindow
				.SetFontOnHierarchy(SystemFont);
		}

		App(const App&) = delete;
		App& operator=(const App&) = delete;

	public:
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
		inline static auto CommonControlsInitialized = 
			[] -> bool
			{
				Win32::InitCommonControls();
				return true;
			}();

		MainWindow m_mainWindow{ 
			CustomWindowArgs{
				.Title = L"Progress Bar Samples",
				.X = Win32::CreateWindowConstants::UseDefault,
				.Y = Win32::CreateWindowConstants::UseDefault,
				.Width = 800,
				.Height = 600,
				.Parent = nullptr
			},
			TopLevelWindowCallbacks{
				.OnResized = 
					[this](unsigned width, unsigned height)
					{
						m_panel.ResizeToParent(0, 0, 150, 0);
						m_statusBar.Resize();
					}
			}
		};

		PanelWindow m_panel{ 
			CustomWindowArgs{
				.X = 0,
				.Y = 0,
				.Width = 800,
				.Height = 500,
				.Parent = m_mainWindow.GetHwnd()
			}
		};

		ProgressBarControl m_progressBar{ m_panel.GetHwnd(), L"", 10, 10, 300, 30 };

		ButtonControl m_button{ 
			m_panel.GetHwnd(),
			L"Click Me", 
			10, 50, 
			100, 30, 
			[this] { m_progressBar.AdvanceIncrement(); } 
		};
		
		StatusBarControl m_statusBar{ m_mainWindow.GetHwnd(), L"", 0, 0, 800, 20 };
	};
}