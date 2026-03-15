export module formbuilder:loader;
import std;
import :win32;
import :schema;
import :events;

export namespace FormDesigner
{
	// Recursively creates child controls for a given parent HWND.
	void CreateChildren(
		Win32::HWND parent,
		Win32::HINSTANCE hInstance,
		std::span<const Control> controls)
	{
		for (auto& control : controls)
		{
			const auto* className = ClassNameFor(control.type);
			if (not className)
			{
				std::println(std::cerr, "Unknown control type, skipping");
				continue;
			}

			auto childStyle = Win32::DWORD{
				Win32::Styles::Child | Win32::Styles::Visible | ImpliedStyleFor(control.type) | control.style
			};

			auto hwnd = Win32::CreateWindowExW(
				control.exStyle,
				className,
				control.text.c_str(),
				childStyle,
				control.rect.x,
				control.rect.y,
				control.rect.width,
				control.rect.height,
				parent,
				reinterpret_cast<Win32::HMENU>(static_cast<Win32::INT_PTR>(control.id)),
				hInstance,
				nullptr
			);

			if (not hwnd)
			{
				std::println(std::cerr, "Failed to create control '{}': error {}",
					std::string(control.text.begin(), control.text.end()),
					Win32::GetLastError());
				continue;
			}

			// Set the default GUI font on the control.
			auto font = static_cast<Win32::HFONT>(Win32::GetStockObject(Win32::DefaultGuiFont));
			Win32::SendMessageW(hwnd, Win32::Messages::SetFont, reinterpret_cast<Win32::WPARAM>(font), 1);

			if (not control.children.empty())
				CreateChildren(hwnd, hInstance, control.children);
		}
	}

	// Window procedure for designer-created forms.
	auto __stdcall FormWndProc(Win32::HWND hwnd, Win32::UINT msg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
	{
		switch (msg)
		{
		case Win32::Messages::Command:
		{
			auto notificationCode = Win32::GetHighWord(wParam);
			if (notificationCode == Win32::Notifications::ButtonClicked)
			{
				auto* events = reinterpret_cast<const EventMap*>(Win32::GetWindowLongPtrW(hwnd, Win32::Gwlp_UserData));
				if (events)
				{
					auto controlId = static_cast<int>(Win32::GetLowWord(wParam));
					if (auto* handler = events->findClickHandler(controlId))
					{
						auto event = ClickEvent{
							.controlId = controlId,
							.controlHwnd = reinterpret_cast<Win32::HWND>(lParam),
							.formHwnd = hwnd,
						};
						(*handler)(event);
						return 0;
					}
				}
			}
			return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
		}
		case Win32::Messages::Destroy:
			Win32::PostQuitMessage(0);
			return 0;
		default:
			return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
		}
	}

	// Creates and shows a top-level window from a Form definition.
	// Returns the HWND of the created window, or nullptr on failure.
	auto LoadForm(const Form& form, Win32::HINSTANCE hInstance, const EventMap& events) -> Win32::HWND
	{
		static constexpr auto ClassName = L"FormDesignerWindow";
		static auto registered = false;

		if (not registered)
		{
			auto wc = Win32::WNDCLASSEXW{
				.cbSize = sizeof(Win32::WNDCLASSEXW),
				.style = Win32::Cs_HRedraw | Win32::Cs_VRedraw,
				.lpfnWndProc = FormWndProc,
				.hInstance = hInstance,
				.hCursor = Win32::LoadCursorW(nullptr, Win32::Cursors::Arrow),
				.hbrBackground = reinterpret_cast<Win32::HBRUSH>(Win32::ColorWindow + 1),
				.lpszClassName = ClassName,
			};

			if (not Win32::RegisterClassExW(&wc))
			{
				std::println(std::cerr, "Failed to register form class: error {}", Win32::GetLastError());
				return nullptr;
			}
			registered = true;
		}

		// Calculate window rect so client area matches requested size.
		auto rc = Win32::RECT{ 0, 0, form.width, form.height };
		Win32::AdjustWindowRectEx(&rc, form.style, 0, form.exStyle);

		auto hwnd = Win32::CreateWindowExW(
			form.exStyle,
			ClassName,
			form.title.c_str(),
			form.style,
			Win32::Cw_UseDefault,
			Win32::Cw_UseDefault,
			rc.right - rc.left,
			rc.bottom - rc.top,
			nullptr,
			nullptr,
			hInstance,
			nullptr
		);

		if (not hwnd)
		{
			std::println(std::cerr, "Failed to create form window: error {}", Win32::GetLastError());
			return nullptr;
		}

		CreateChildren(hwnd, hInstance, form.controls);

		// Store event map pointer for WndProc dispatch.
		Win32::SetWindowLongPtrW(hwnd, Win32::Gwlp_UserData, reinterpret_cast<Win32::LONG_PTR>(&events));

		Win32::ShowWindow(hwnd, Win32::Sw_ShowDefault);
		Win32::UpdateWindow(hwnd);

		return hwnd;
	}

	// Runs the message loop. Call after LoadForm.
	auto RunMessageLoop() -> int
	{
		auto msg = Win32::MSG{};
		while (Win32::GetMessageW(&msg, nullptr, 0, 0) > 0)
		{
			Win32::TranslateMessage(&msg);
			Win32::DispatchMessageW(&msg);
		}
		return static_cast<int>(msg.wParam);
	}
}
