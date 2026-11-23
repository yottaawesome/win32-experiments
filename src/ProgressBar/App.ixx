export module ProgressBarSamples:App;
import std;
import Win32;
import :MainWindow;
import :ProgressBarControl;

export namespace ProgressBarSamples
{
	class App
	{
	public:
		auto Run() -> int
		{
			m_progressBar.SetRange(0, 100);
			m_progressBar.SetPosition(0);
			m_progressBar.SetIncrement(10);

			auto updateThread = std::jthread{ 
				[this] 
				{ 
					std::this_thread::sleep_for(std::chrono::seconds{ 3 });
					m_progressBar.SetPosition(50);
					std::this_thread::sleep_for(std::chrono::seconds{ 3 });
					m_progressBar.AdvanceIncrement();
					std::this_thread::sleep_for(std::chrono::seconds{ 3 });
					m_progressBar.SetMarquee(true, 20);
				}
			};

			auto msg = Win32::MSG{};
			while (Win32::GetMessageW(&msg, nullptr, 0, 0))
			{
				Win32::TranslateMessage(&msg);
				Win32::DispatchMessageW(&msg);
			}
			return static_cast<int>(msg.wParam);
		}

	private:
		MainWindow m_mainWindow;
		ProgressBarControl m_progressBar{ m_mainWindow.GetHwnd(), 10, 10, 300, 30 };
	};
}