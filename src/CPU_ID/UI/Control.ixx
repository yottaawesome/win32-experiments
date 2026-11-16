export module CpuId:UI.Control;
import std;
import Win32;
import :Error;

export namespace UI
{
    template<typename TImplementation>
    class Control
    {
    public:
		~Control()
		{
			Destroy();
		}

		Control() = default;

		Control(const Control&) = delete;
		auto operator=(const Control&) -> Control & = delete;

        void Destroy()
        {
            if (not m_hwnd)
                return;
            auto tmp = m_hwnd;
            m_hwnd = nullptr;
            Win32::RemoveWindowSubclass(tmp, &Control::SubclassProc, 0);
            Win32::DestroyWindow(tmp);
        }

		auto release() -> void
		{
			if (m_hwnd)
			{
				Win32::SetWindowLongPtrW(m_hwnd, Win32::Gwlp::UserData, 0);
				m_hwnd = nullptr;
			}
		}

        [[nodiscard]]
        auto GetHwnd() const noexcept -> Win32::HWND
        {
            return m_hwnd;
        }

    protected:
        static auto SubclassProc(
            Win32::HWND hwnd,
            unsigned uMsg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam,
            Win32::UINT_PTR,
            Win32::DWORD_PTR dwRefData
        ) -> Win32::LRESULT
        {
            if (auto pThis = reinterpret_cast<TImplementation*>(dwRefData); pThis)
                if (auto result = pThis->HandleMessage(hwnd, uMsg, wParam, lParam); result)
                    return *result;
            return Win32::DefSubclassProc(hwnd, uMsg, wParam, lParam);
        }

        void Create(
            unsigned controlId,
            std::wstring_view className,
            const std::wstring& text,
            Win32::DWORD styles,
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
                className.data(),
                text.c_str(),
                styles,
                x, y,
                width, height,
                parent,
                reinterpret_cast<Win32::HMENU>(static_cast<Win32::ULONG_PTR>(controlId)),
                Win32::GetModuleHandleW(nullptr),
                nullptr);
            if (not m_hwnd)
                throw Error::Win32Error{ Win32::GetLastError(), "CreateWindowExW failed" };
            if (not Win32::SetWindowSubclass(m_hwnd, &SubclassProc, 0, reinterpret_cast<Win32::DWORD_PTR>(this)))
                throw std::runtime_error{ "Failed to track control window destruction" };
        }

        Win32::HWND m_hwnd = nullptr;
    };
}
