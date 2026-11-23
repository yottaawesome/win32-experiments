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

		// The subclass callback retains this object's address.
		ControlWindow(const ControlWindow&) = delete;
		auto operator=(const ControlWindow&) -> ControlWindow& = delete;
		ControlWindow(ControlWindow&&) = delete;
		auto operator=(ControlWindow&&) -> ControlWindow& = delete;

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
			if (not Win32::SetWindowSubclass(self.m_hwnd, ControlProc, 0, reinterpret_cast<Win32::DWORD_PTR>(&self)))
			{
				Win32::DestroyWindow(std::exchange(self.m_hwnd, nullptr));
				throw Error::RuntimeError{ "Failed to subclass control window" };
			}
			return self;
		}

	private:
		static auto __stdcall ControlProc(
			Win32::HWND hwnd,
			Win32::UINT msg,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam,
			Win32::UINT_PTR subclassId,
			Win32::DWORD_PTR dwRefData
		) -> Win32::LRESULT
		{
			auto ptr = reinterpret_cast<ControlWindow*>(dwRefData);
			if (msg == Win32::WindowMessages::NonClientDestroy)
			{
				if (ptr)
					ptr->m_hwnd = nullptr;
				Win32::RemoveWindowSubclass(hwnd, ControlProc, subclassId);
				return Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
			}
			return ptr
				? ptr->HandleMessage(hwnd, msg, wParam, lParam)
				: Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
		}

	protected:
		Win32::HWND m_hwnd = nullptr;
	};
}