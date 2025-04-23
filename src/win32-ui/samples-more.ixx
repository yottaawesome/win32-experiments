export module samples:more;
import std;
import shared;
import util;

export namespace ChildControls
{
	template<Util::FixedString VRegistrationName, Win32::WNDPROC VProc>
	struct WindowRegistration
	{
		~WindowRegistration()
		{
			Win32::UnregisterClassW(VRegistrationName.Data(), m_instance);
		}

		WindowRegistration(bool doRegistration)
		{
			if (doRegistration)
				Register();
		}

		void Register()
		{
			Win32::WNDCLASSEX wc{
				.cbSize = sizeof(wc),
				.lpfnWndProc = VProc,
				.hInstance = m_instance,
				.lpszClassName = VRegistrationName.Data()
			};

			m_registration = Win32::RegisterClassExW(&wc);
			if (not m_registration)
				throw Error::Win32Error(Win32::GetLastError(), "Failed registering class {}", Util::ConvertString(VRegistrationName.Data()));
		}

		Win32::ATOM m_registration = 0;
		Win32::HMODULE m_instance = Win32::GetModuleHandleW(nullptr);
	};

	struct SimpleWindow
	{
		using HwndSharedPtr = std::shared_ptr<std::remove_pointer_t<Win32::HWND>>;
		SimpleWindow(Win32::HWND hwnd) : Handle(hwnd) {}
		HwndSharedPtr Handle;
	};

	struct Control
	{

	};

	struct MainWindow
	{

	};

	void Run()
	{
		using MainWindowReg = WindowRegistration<
			L"A", 
			[](Win32::HWND, unsigned, Win32::WPARAM, Win32::LPARAM) -> Win32::LPARAM { return 0; }
		>;

		MainWindowReg registration(false);
	}
}
