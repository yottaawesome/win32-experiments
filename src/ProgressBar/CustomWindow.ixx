export module MultiControlSamples:CustomWindow;
import std;
import Win32;
import :Error;
import :WindowCommon;

export namespace MultiControlSamples
{
	template<typename TWindow> // GetWindowClassName() must be defined in TWindow
	class CustomWindow : public TWindow
	{
	public:
		~CustomWindow()
		{
			if (m_hwnd)
				Win32::DestroyWindow(m_hwnd);
		}

		CustomWindow(int x, int y, unsigned width, unsigned height, Win32::HWND parent, auto&&...additionalArgs)
			: TWindow{ std::forward<decltype(additionalArgs)>(additionalArgs)... }
		{
			Initialize({
				.ClassName = std::wstring{ this->GetWindowClassName() },
				.Title = L"Progress Bar Samples",
				.ExtendedStyle = this->GetDefaultExtendedStyle(),
				.Style = this->GetDefaultStyle(),
				.X = x,
				.Y = y,
				.Width = width,
				.Height = height,
				.Parent = parent
			});
		}

		//
		// No copy semantics
		CustomWindow(const CustomWindow&) = delete;
		auto operator=(const CustomWindow&) -> CustomWindow & = delete;

		//
		// Move semantics
		CustomWindow(CustomWindow&& other) noexcept
			: m_hwnd{ std::exchange(other.m_hwnd, nullptr) }
		{
			// other m_hwnd could be nullptr
			if (m_hwnd)
				Win32::SetWindowLongPtrW(m_hwnd, Win32::Gwlp::UserData, reinterpret_cast<Win32::LONG_PTR>(static_cast<TWindow*>(this)));
		}
		auto operator=(CustomWindow&& other) noexcept -> CustomWindow&
		{
			if (this == &other)
				return *this;
			if (m_hwnd)
				Win32::DestroyWindow(m_hwnd);
			// other m_hwnd could be nullptr
			if (m_hwnd = std::exchange(other.m_hwnd, nullptr); m_hwnd)
				Win32::SetWindowLongPtrW(m_hwnd, Win32::Gwlp::UserData, reinterpret_cast<Win32::LONG_PTR>(static_cast<TWindow*>(this)));
			return *this;
		}

		//
		// Public methods
	public:
		[[nodiscard]]
		auto GetHwnd(this auto&& self) noexcept -> Win32::HWND
		{
			return self.m_hwnd;
		}

	protected:
		auto Initialize(this auto&& self, const CreateWindowArgs& args) -> decltype(self)
		{
			if (self.m_hwnd)
				return self;

			self.RegisterClass().CreateWindow(args);
			return self;
		}

		auto RegisterClass(this auto&& self) -> decltype(self)
		{
			using TWindowInstance = std::remove_cvref_t<decltype(self)>;

			constexpr auto WndProc =
				[](Win32::HWND hwnd, Win32::UINT msg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
				{
					switch (msg)
					{
						// Attach the window instance to the window handle on creation, and detach it on destruction
					case Win32::WindowMessages::NonClientCreate:
					{
						auto createStruct = reinterpret_cast<Win32::CREATESTRUCTW*>(lParam);
						if (auto ptr = reinterpret_cast<TWindowInstance*>(createStruct->lpCreateParams); ptr)
						{
							Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp::UserData, reinterpret_cast<Win32::LONG_PTR>(ptr));
							ptr->m_hwnd = hwnd;
						}
						return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
					}

					// Detach the window instance from the window handle when the window is destroyed
					case Win32::WindowMessages::NonClientDestroy:
					{
						if (auto ptr = reinterpret_cast<TWindowInstance*>(Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp::UserData)); ptr)
						{
							ptr->m_hwnd = nullptr;
							Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp::UserData, 0);
						}
						return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
					}

					default:
					{
						auto ptr = reinterpret_cast<TWindowInstance*>(Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp::UserData));
						return ptr
							? ptr->HandleMessage(hwnd, msg, wParam, lParam)
							: Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
					}
					}
				};

			auto wndclassEx = Win32::WNDCLASSEXW{
				.cbSize = sizeof(Win32::WNDCLASSEXW),
				.lpfnWndProc = WndProc,
				.hInstance = Win32::GetModuleHandleW(nullptr),
				.hCursor = Win32::LoadCursorW(nullptr, Win32::IdcArrow),
				.hbrBackground = (Win32::HBRUSH)(Win32::ColorWindow + 1),
				.lpszClassName = self.GetWindowClassName().data()
			};
			if (not Win32::RegisterClassExW(&wndclassEx))
				if (auto errorCode = Win32::GetLastError(); errorCode != Win32::ErrorCodes::ClassAlreadyExists)
					throw Error::Win32Error{ errorCode, "Failed to register window class" };

			return self;
		}

		auto CreateWindow(this auto&& self, const CreateWindowArgs& args) -> decltype(self)
		{
			if (self.m_hwnd)
				return self;
			Win32::CreateWindowExW(
				args.ExtendedStyle,
				self.GetWindowClassName().data(),
				args.Title.data(),
				args.Style,
				args.X,
				args.Y,
				args.Width,
				args.Height,
				args.Parent,
				nullptr,
				Win32::GetModuleHandleW(nullptr),
				&self
			);
			if (not self.m_hwnd)
				throw Error::Win32Error{ Win32::GetLastError(), "Failed to create window" };
			return self;
		}

	protected:
		Win32::HWND m_hwnd = nullptr;
	};
}
