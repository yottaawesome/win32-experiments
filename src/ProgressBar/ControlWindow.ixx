export module MultiControlSamples:ControlWindow;
import std;
import Win32;
import :WindowCommon;
import :Error;

export namespace MultiControlSamples
{
	template<typename T>
	concept ControlBaseType = requires(T t)
	{
		{ t.GetDefaultParams() } -> std::same_as<ControlParams>;
		{ t.HandleMessage(
			std::declval<Win32::HWND>(), 
			std::declval<Win32::UINT>(), 
			std::declval<Win32::WPARAM>(), 
			std::declval<Win32::LPARAM>()) 
		} -> std::same_as<Win32::LRESULT>;
	};

	template<ControlBaseType TDerived>
	class ControlWindow : public TDerived
	{
	public:
		~ControlWindow()
		{
			if (m_hwnd)
				Win32::DestroyWindow(m_hwnd);
		}

		// No copy semantics
		ControlWindow(const ControlWindow&) = delete;
		auto operator=(const ControlWindow&) -> ControlWindow & = delete;

		// Move semantics
		ControlWindow(ControlWindow&& other) noexcept
			: m_hwnd{ std::exchange(other.m_hwnd, nullptr) }
		{
			// other m_hwnd could be nullptr
			if (m_hwnd)
				SetSubclassProc();
		}
		auto operator=(ControlWindow&& other) noexcept -> ControlWindow&
		{
			if (this == &other)
				return *this;
			if (m_hwnd)
				Win32::DestroyWindow(m_hwnd);
			// other m_hwnd could be nullptr
			if (m_hwnd = std::exchange(other.m_hwnd, nullptr); m_hwnd)
				SetSubclassProc();
			return *this;
		}

		ControlWindow(
			Win32::HWND parent, 
			const std::wstring& title, 
			int x, int y, 
			unsigned width, unsigned height,
			auto&&... args
		) : TDerived{ std::forward<decltype(args)>(args)... }
		{
			auto defaultParams = this->GetDefaultParams();
			Initialize(
				CreateControlArgs{
					.ControlId = NextControlId(),
					.ClassName = defaultParams.ClassName,
					.Title = title,
					.ExtendedStyle = defaultParams.ExtendedStyle,
					.Style = defaultParams.Style,
					.X = x,
					.Y = y,
					.Width = width,
					.Height = height,
					.Parent = parent,
				});
		}

	public:
		[[nodiscard]]
		auto GetHwnd(this auto&& self) noexcept -> Win32::HWND
		{
			return self.m_hwnd;
		}

	protected:
		auto Initialize(this auto&& self, const CreateControlArgs& args) -> decltype(self)
		{
			self.m_hwnd = Win32::CreateWindowExW(
				args.ExtendedStyle,
				args.ClassName.data(),
				args.Title.data(),
				Win32::WindowStyles::WsChild | Win32::WindowStyles::WsVisible | args.Style,
				args.X,
				args.Y,
				args.Width,
				args.Height,
				args.Parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::ULONG_PTR>(args.ControlId)),
				Win32::GetModuleHandleW(nullptr),
				&self
			);
			if (not self.m_hwnd)
				throw Error::Win32Error{ Win32::GetLastError(), "Failed to create window" };
			self.SetSubclassProc();
			return self;
		}

		auto SetSubclassProc(this auto&& self)
		{
			constexpr auto ControlProc =
				[](
					Win32::HWND hwnd,
					Win32::UINT msg,
					Win32::WPARAM wParam,
					Win32::LPARAM lParam,
					Win32::UINT_PTR,
					Win32::DWORD_PTR dwRefData
				) -> Win32::LRESULT
				{
					auto ptr = reinterpret_cast<ControlWindow*>(dwRefData);
					if (not ptr)
						return Win32::DefSubclassProc(hwnd, msg, wParam, lParam);

					switch (msg)
					{
						case Win32::WindowMessages::NonClientDestroy:
						{
							ptr->m_hwnd = nullptr;
							Win32::SetWindowSubclass(hwnd, nullptr, 0, 0);
							return Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
						}

						default:
						{
							return ptr->HandleMessage(hwnd, msg, wParam, lParam);
						}
					}
				};

			Win32::SetWindowSubclass(self.m_hwnd, ControlProc, 0, reinterpret_cast<Win32::DWORD_PTR>(&self));
		}

	protected:
		Win32::HWND m_hwnd = nullptr;
	};
}