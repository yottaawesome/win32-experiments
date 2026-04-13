export module uicallbacks:control;
import std;
import :win32;
import :error;
import :window;

export namespace UiCallbacks::UI
{
    // This is the common idiom for handling button clicks.
    class Button
    {
    public:
        std::move_only_function<auto()->void> OnClick;

        ~Button()
        {
            parent.RemoveCommand(id);
            if (hwnd)
                Win32::DestroyWindow(hwnd);
        }

        Button(const Button&) = delete;
        auto operator=(const Button&) -> Button & = delete;

        Button(Window& parentIn, Win32::WORD id, std::wstring_view text, int x, int y, int w, int h)
            : parent(parentIn), id(id)
        {
			Init(id, text, x, y, w, h);
        }

        auto Init(this auto&& self, Win32::WORD id, std::wstring_view text, int x, int y, int w, int h)
        {
            self.hwnd = Create(id, text, x, y, w, h, self.parent.Handle());
            // register the command handler on the parent
            self.parent.OnCommand(
                id,
                [&self](Win32::WORD notify) -> Win32::LRESULT
                {
                    if (notify == Win32::ButtonMessages::Clicked and self.OnClick)
                        self.OnClick();
                    return 0;
                });
        }

        static auto Create(Win32::WORD id, std::wstring_view text, int x, int y, int w, int h, Win32::HWND parent) -> Win32::HWND
        {
            auto hwnd = Win32::CreateWindowExW(
                0,
                L"BUTTON",
                text.data(),
                Win32::WindowStyles::Child | Win32::WindowStyles::Visible,
                x, y, w, h,
                parent,
                reinterpret_cast<HMENU>(id),
                nullptr,
                nullptr
            );
            if (not hwnd)
                throw UiCallbacks::Error::Win32Error{ Win32::GetLastError(), "Failed to create button" };
            return hwnd;
        }


        auto Enable(bool enabled) -> void { Win32::EnableWindow(hwnd, enabled); }
        auto SetText(std::wstring_view text) -> void { Win32::SetWindowTextW(hwnd, text.data()); }

    private:
        Window& parent;
        int id = 0;
        Win32::HWND hwnd = nullptr;
        std::move_only_function<void()> onClick;
    };

    // This is uncommon purely for message handling.
    class SubclassButton
    {
    public:
        template<typename TControl>
        static auto SubclassProc(
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

        SubclassButton(const SubclassButton&) = delete;
		auto operator=(const SubclassButton&) -> SubclassButton& = delete;

        SubclassButton(Window& parentIn, Win32::WORD id, std::wstring_view text, int x, int y, int w, int h)
			: parent(parentIn), id(id)
        {
            Create(id, text, x, y, w, h, parent.Handle());
        }

		std::move_only_function<auto()->void> OnClick;

        auto HandleMessage(
            this auto&& self,
            Win32::HWND hwnd,
            Win32::UINT msg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam,
            Win32::UINT_PTR idSubclass,
            Win32::DWORD_PTR refData
        ) -> Win32::LRESULT
        {
            switch (msg)
            {
            case Win32::WindowMessages::LeftButtonUp:
                if (self.OnClick)
                    self.OnClick();
                return 0;
            }

            if (msg == Win32::WindowMessages::NonClientDestroy)
                Win32::RemoveWindowSubclass(hwnd, SubclassProc<std::remove_cvref_t<decltype(self)>>, self.GetSubclassId());

            return Win32::DefSubclassProc(hwnd, msg, wParam, lParam);
		}

        auto SetWindowSubclass(this auto&& self) -> void
        {
            if(not Win32::SetWindowSubclass(self.hwnd, SubclassProc<std::remove_cvref_t<decltype(self)>>, self.GetSubclassId(), reinterpret_cast<Win32::DWORD_PTR>(&self)))
                throw UiCallbacks::Error::Win32Error{ Win32::GetLastError(), "Failed to set window subclass" };
		}

        auto Create(this auto&& self, Win32::WORD id, std::wstring_view text, int x, int y, int w, int h, Win32::HWND parent) -> Win32::HWND
        {
            self.hwnd = Win32::CreateWindowExW(
                0,
                L"BUTTON",
                text.data(),
                Win32::WindowStyles::Child | Win32::WindowStyles::Visible,
                x, y, w, h,
                parent,
                reinterpret_cast<HMENU>(id),
                nullptr,
                nullptr
            );
            if (not self.hwnd)
                throw UiCallbacks::Error::Win32Error{ Win32::GetLastError(), "Failed to create button" };

			self.SetWindowSubclass();

            return self.hwnd;
        }

        auto GetSubclassId(this auto&& self) -> Win32::UINT_PTR
        {
            return static_cast<Win32::UINT_PTR>(self.id);
		}

	private:
		Window& parent;
		Win32::HWND hwnd = nullptr;
        int id = 0;
    };
}
