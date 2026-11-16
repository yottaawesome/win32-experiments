export module CpuId:UI.Label;
import std;
import Win32;
import :Error;
import :UI.Control;

export namespace UI
{
    class Label : public Control<Label>
    {
    public:
        ~Label()
        {
            Destroy();
        }

		void Destroy()
		{
            if (not m_hwnd)
                return;
			auto tmp = m_hwnd;
			m_hwnd = nullptr;
			Win32::RemoveWindowSubclass(tmp, &Control<Label>::SubclassProc, 0);
			Win32::DestroyWindow(tmp);
		}

        Label(const Label&) = delete;
        auto operator=(const Label&) -> Label & = delete;

        Label(Label&& other) noexcept
            : m_hwnd{ other.m_hwnd }
        {
            other.m_hwnd = nullptr;
            if (not Win32::SetWindowSubclass(m_hwnd, &SubclassProc, 0, reinterpret_cast<Win32::DWORD_PTR>(this)))
                throw std::runtime_error{ "Failed to track control window destruction" };
        }
		auto operator=(Label&& other) noexcept -> Label&
		{
			if (this == &other)
				return *this;
			Destroy();
			m_hwnd = std::exchange(other.m_hwnd, nullptr);
			Win32::SetWindowSubclass(m_hwnd, &SubclassProc, 0, reinterpret_cast<Win32::DWORD_PTR>(this));
			return *this;
		}

        Label(
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

        auto HandleMessage(Win32::HWND hwnd, unsigned uMsg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> std::optional<Win32::LRESULT>
        {
            if (uMsg == Win32::Messages::NonClientDestroy)
            {
                Win32::RemoveWindowSubclass(hwnd, &Control<Label>::SubclassProc, 0);
                m_hwnd = nullptr;
            }
            return std::nullopt;
        }

        [[nodiscard]]
        auto GetHwnd() const noexcept -> Win32::HWND
        {
            return m_hwnd;
        }

        auto SetText(const std::wstring& text)
        {
            if (not m_hwnd)
                throw std::runtime_error{ "Label window handle is null" };
            Win32::SetWindowTextW(m_hwnd, text.c_str());
        }

        auto SetText(bool value)
        {
            if (not m_hwnd)
                throw std::runtime_error{ "Label window handle is null" };
            Win32::SetWindowTextW(m_hwnd, value ? L"True" : L"False");
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
            m_hwnd = Win32::CreateWindowExW(
                0,
                L"STATIC",
                text.c_str(),
                Win32::WindowStyles::Child | Win32::WindowStyles::Visible | additionalStyles,
                x, y,
                width, height,
                parent,
                reinterpret_cast<Win32::HMENU>(static_cast<Win32::ULONG_PTR>(controlId)),
                Win32::GetModuleHandleW(nullptr),
                nullptr);
            if (not hwnd)
                throw Error::Win32Error{ Win32::GetLastError(), "CreateWindowExW failed" };
            if (not Win32::SetWindowSubclass(hwnd, &SubclassProc, 0, reinterpret_cast<Win32::DWORD_PTR>(this)))
                throw std::runtime_error{ "Failed to track control window destruction" };
        }

        Win32::HWND m_hwnd = nullptr;
    };
}