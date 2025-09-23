export module app:mainwindow;
import std;
import :common;
import :win32;
import :toplevelwindow;
import :controls;
import :painting;

export namespace UI
{
	struct MainWindow : TopLevelWindow
	{
		static constexpr std::wstring_view ClassName = L"SampleAppMainWindow";

		auto GetClass(this auto&& self) -> Win32::WNDCLASSEXW 
		{
			return Win32::WNDCLASSEXW{
				.cbSize = sizeof(Win32::WNDCLASSEXW),
				.hInstance = Win32::GetModuleHandleW(nullptr),
				.hIcon = Win32::LoadIconW(nullptr, Win32::IdiApplication),
				.hCursor = Win32::LoadCursorW(nullptr, Win32::IdcArrow),
				.hbrBackground = BlackBrush,
				.lpszClassName = self.ClassName.data()
			};
		}

		auto GetCreationArgs(this auto&& self) -> CreateWindowArgs
		{
			return {
				.WindowName = L"Win32 Calculator",
				// disable resize
				.Style = Win32::WindowStyles::WsOverlappedWindow & ~Win32::WindowStyles::ThickFrame,
				.Width = 750,
				.Height = 750
			};
		}

		void Init(this auto&& self)
		{
			self.m_button.Create(self.m_window.get());
		}

		auto OnMessage(this auto&& self, Win32Message<Win32::Messages::Paint> msg) -> Win32::LRESULT
		{
			return Win32::DefWindowProcW(msg.Hwnd, msg.uMsg, msg.wParam, msg.lParam);
		}

		OwnerDrawnButton m_button;
	};
}
