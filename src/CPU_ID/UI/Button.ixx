export module CpuId:UI.Button;
import std;
import Win32;
import :Error;
import :UI.Control;

export namespace UI
{
	class Button : public Control<Button>
	{
	public:
		~Button()
		{
			Destroy();
		}

		Button() = default;

		Button(
			unsigned controlId,
			const std::wstring& text,
			Win32::HWND parent,
			unsigned x,
			unsigned y,
			unsigned width,
			unsigned height,
			Win32::DWORD additionalStyles = 0
		)
		{
			if (not parent)
				throw std::invalid_argument{ "parent window handle is null" };
			Create(controlId, text, parent, x, y, width, height, additionalStyles);
		}

		//
		// Copy
		Button(const Button&) = delete;
		auto operator=(const Button&) -> Button & = delete;

		//
		// Move
		Button(Button&& other) noexcept
			: m_hwnd{ other.m_hwnd }
		{
			other.m_hwnd = nullptr;
			if (not Win32::SetWindowSubclass(m_hwnd, &SubclassProc, 0, reinterpret_cast<Win32::DWORD_PTR>(this)))
				throw std::runtime_error{ "Failed to track control window destruction" };
		}
		auto operator=(Button&& other) noexcept -> Button&
		{
			if (this == &other)
				return *this;
			Destroy();
			m_hwnd = std::exchange(other.m_hwnd, nullptr);
			Win32::SetWindowSubclass(m_hwnd, &SubclassProc, 0, reinterpret_cast<Win32::DWORD_PTR>(this));
			return *this;
		}

		auto GetHwnd() const noexcept -> Win32::HWND
		{
			return m_hwnd;
		}

		void Destroy()
		{
			if (m_hwnd)
			{
				auto tmp = m_hwnd;
				release();
				Win32::DestroyWindow(tmp);
			}
		}

		void release()
		{
			if (m_hwnd)
			{
				Win32::SetWindowLongPtrW(m_hwnd, Win32::Gwlp::UserData, 0);
				m_hwnd = nullptr;
			}
		}

		void OnClick()
		{
			int x = 10;
		}

		[[nodiscard]]
		auto HandleMessage(
			Win32::HWND hwnd,
			unsigned uMsg,
			Win32::WPARAM wParam,
			Win32::LPARAM lParam
		) -> std::optional<Win32::LRESULT>
		{
			switch (uMsg)
			{
				case Win32::Messages::ReflectedCommand:
				{
					if (Win32::HiWord(wParam) == Win32::BN::Clicked)
					{
						OnClick();
					}
					// Handle reflected command messages here if needed
					return 0;
				}

				case Win32::Messages::Destroy:
				{
					release();
					return 0;
				}

				default:
					return std::nullopt;
			}
		}

	protected:
		void Create(
			unsigned controlId,
			const std::wstring& text,
			Win32::HWND parent,
			unsigned x,
			unsigned y,
			unsigned width,
			unsigned height,
			Win32::DWORD additionalStyles = 0
		)
		{
			if (not parent)
				throw std::invalid_argument{ "parent window handle is null" };
			m_hwnd = Win32::CreateWindowExW(
				0,
				L"BUTTON",
				text.c_str(),
				Win32::WindowStyles::Child | Win32::WindowStyles::Visible | additionalStyles,
				static_cast<int>(x),
				static_cast<int>(y),
				static_cast<int>(width),
				static_cast<int>(height),
				parent,
				reinterpret_cast<Win32::HMENU>(static_cast<std::uintptr_t>(controlId)),
				Win32::GetModuleHandleW(nullptr),
				this);
			if (not m_hwnd)
				throw Error::Win32Error{ Win32::GetLastError(), "Failed to create button control" };
			if (not Win32::SetWindowSubclass(m_hwnd, &SubclassProc, 0, reinterpret_cast<Win32::DWORD_PTR>(this)))
				throw std::runtime_error{ "Failed to track control window destruction" };
		}

		Win32::HWND m_hwnd = nullptr;
	};
}