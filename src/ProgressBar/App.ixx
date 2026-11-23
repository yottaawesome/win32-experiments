export module MultiControlSamples:App;
import std;
import Win32;
import :CustomWindow;
import :TopLevelWindow;
import :ProgressBarControl;
import :ButtonControl;
import :StatusBarControl;
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
			L"Progress Bar Samples",
			Win32::CreateWindowConstants::UseDefault, 
			Win32::CreateWindowConstants::UseDefault,
			800,
			600,
			nullptr,
			TopLevelWindowCallbacks{
				.OnResized = 
					[this](unsigned width, unsigned height)
					{
						m_statusBar.Resize();
					}
			}
		};

		ProgressBarControl m_progressBar{ m_mainWindow.GetHwnd(), L"", 10, 10, 300, 30 };

		ButtonControl m_button{ 
			m_mainWindow.GetHwnd(), 
			L"Click Me", 
			10, 50, 
			100, 30, 
			[this] { m_progressBar.AdvanceIncrement(); } 
		};
		
		StatusBarControl m_statusBar{ m_mainWindow.GetHwnd(), L"", 0, 0, 800, 20 };
	};
}