export module Samples:LatestSample;
import std;
import Shared;

export namespace LatestSample
{
	template<auto VDeleteFn>
    struct Deleter
    {
        static constexpr void operator()(Win32::HWND hwnd) noexcept
        {
            VDeleteFn(hwnd);
        }
	};

	using HwndUniquePtr = std::unique_ptr<std::remove_pointer_t<Win32::HWND>, Deleter<Win32::DestroyWindow>>;

	struct SizeMessage
	{
		// https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-size
		enum class Type : Win32::WPARAM
		{
			Restored  = 0, // SIZE_RESTORED
			Minimized = 1, // SIZE_MINIMIZED
			Maximized = 2, // SIZE_MAXIMIZED
			MaxShow   = 3, // SIZE_MAXSHOW
			MaxHide   = 4, // SIZE_MAXHIDE
		};

		Type type;
		int width;
		int height;

		static constexpr auto From(Win32::WPARAM wParam, Win32::LPARAM lParam) noexcept -> SizeMessage
		{
			auto [width, height] = Win32::SplitWord(lParam);
			return { static_cast<Type>(wParam), width, height };
		}
	};

    struct MainWindow
    {
        HwndUniquePtr WindowHandle;

        MainWindow(HwndUniquePtr hwnd) noexcept
            : WindowHandle(std::move(hwnd))
        {
            if (WindowHandle)
                Win32::SetWindowLongPtrW(WindowHandle.get(), Win32::Gwlp_UserData, reinterpret_cast<Win32::LONG_PTR>(this));
        }

        MainWindow(const MainWindow&) = delete;
        MainWindow& operator=(const MainWindow&) = delete;

        MainWindow(MainWindow&& other) noexcept
            : WindowHandle(std::move(other.WindowHandle))
        {
            if (WindowHandle)
                Win32::SetWindowLongPtrW(WindowHandle.get(), Win32::Gwlp_UserData, reinterpret_cast<Win32::LONG_PTR>(this));
        }

        MainWindow& operator=(MainWindow&& other) noexcept
        {
            if (this != &other)
            {
                WindowHandle = std::move(other.WindowHandle);
                if (WindowHandle)
                    Win32::SetWindowLongPtrW(WindowHandle.get(), Win32::Gwlp_UserData, reinterpret_cast<Win32::LONG_PTR>(this));
            }
            return *this;
        }
    };

    template<typename TWindow>
    auto WndProc(Win32::HWND hwnd, Win32::UINT msg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
    {
        auto pThis = reinterpret_cast<TWindow*>(Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData));
        if (not pThis)
            return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);

        if (msg == Win32::Messages::NonClientDestroy and pThis->WindowHandle.get() == hwnd)
        {
            Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData, 0);
            pThis->WindowHandle.release();
            return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
        }

        if constexpr (requires(SizeMessage m) { pThis->OnSize(m); })
        {
            if (msg == Win32::Messages::Size)
                return pThis->OnSize(SizeMessage::From(wParam, lParam));
        }

        return msg == Win32::Messages::Destroy
            ? (Win32::PostQuitMessage(0), 0)
            : Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
    }

	template<typename TWindow>
    auto RegisterWindowClass()
    {
        auto windowClass = Win32::WNDCLASSEXW{
            .cbSize = sizeof(Win32::WNDCLASSEXW),
            .lpfnWndProc = WndProc<TWindow>,
            .hInstance = Win32::GetModuleHandleW(nullptr),
            .lpszClassName = L"AnotherWeirdSampleWindowClass",
        };
        auto result = Win32::RegisterClassExW(&windowClass);
        if (not result)
        {
            std::println("Failed registering class: {}", Win32::GetLastError());
            std::terminate();
        }
        return result;
	}

    template<typename TControl>
    auto SubclassProc(
        Win32::HWND hwnd,
        Win32::UINT msg,
        Win32::WPARAM wParam,
        Win32::LPARAM lParam,
        Win32::UINT_PTR idSubclass,
        Win32::DWORD_PTR refData
    ) -> Win32::LRESULT
    {
        auto pThis = reinterpret_cast<TControl*>(refData);
        return pThis
            ? pThis->HandleMessage(hwnd, msg, wParam, lParam, idSubclass, refData)
            : Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
    }

    auto Run()
    {
		RegisterWindowClass<MainWindow>();

        auto hwnd = Win32::CreateWindowExW(
            0,
            L"AnotherWeirdSampleWindowClass",
            L"Another weird sample",
            Win32::Styles::OverlappedWindow,
            0,
            0,
            300,
            200,
            nullptr,
            nullptr,
            Win32::GetModuleHandleW(nullptr),
            nullptr
        );
        if (not hwnd)
        {
            std::println("Failed creating window: {}", Win32::GetLastError());
            std::terminate();
        }
        auto mainWindow = MainWindow{ HwndUniquePtr{ hwnd } };
        Win32::ShowWindow(hwnd, Win32::Sw_ShowDefault);

        auto msg = Win32::MSG{};
        while (Win32::GetMessageW(&msg, nullptr, 0, 0) > 0)
        {
            Win32::TranslateMessage(&msg);
            Win32::DispatchMessageW(&msg);
        }
		return static_cast<int>(msg.wParam);
	}
}
