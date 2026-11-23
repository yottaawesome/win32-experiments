export module MultiControlSamples:App;
import std;
import Win32;
import :CustomWindow;
import :TopLevelWindow;
import :ProgressBarControl;
import :ButtonControl;

export namespace MultiControlSamples
{
	class App
	{
	public:
		using MainWindow = CustomWindow<TopLevelWindow>;

		auto Run() -> int
		{
			m_progressBar.SetRange(0, 100);
			m_progressBar.SetPosition(0);
			m_progressBar.SetIncrement(10);

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
			Win32::CreateWindowConstants::UseDefault, 
			Win32::CreateWindowConstants::UseDefault,
			800,
			600,
			nullptr
		};
		ProgressBarControl m_progressBar{ m_mainWindow.GetHwnd(), L"", 10, 10, 300, 30 };
		ButtonControl m_button{ m_mainWindow.GetHwnd(), L"Click Me", 10, 50, 100, 30, [this] { m_progressBar.AdvanceIncrement(); } };
	};
}