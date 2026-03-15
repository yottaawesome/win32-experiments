module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>

export module formbuilder:loader;
import std;
import :schema;
import :events;

export namespace FormDesigner
{
	// Recursively creates child controls for a given parent HWND.
	void CreateChildren(
		HWND parent,
		HINSTANCE hInstance,
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

			auto childStyle = DWORD{ WS_CHILD | WS_VISIBLE | ImpliedStyleFor(control.type) | control.style };

			auto hwnd = CreateWindowExW(
				control.exStyle,
				className,
				control.text.c_str(),
				childStyle,
				control.rect.x,
				control.rect.y,
				control.rect.width,
				control.rect.height,
				parent,
				reinterpret_cast<HMENU>(static_cast<INT_PTR>(control.id)),
				hInstance,
				nullptr
			);

			if (not hwnd)
			{
				std::println(std::cerr, "Failed to create control '{}': error {}",
					std::string(control.text.begin(), control.text.end()),
					GetLastError());
				continue;
			}

			// Set the default GUI font on the control.
			auto font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
			SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);

			if (not control.children.empty())
				CreateChildren(hwnd, hInstance, control.children);
		}
	}

	// Window procedure for designer-created forms.
	auto CALLBACK FormWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
	{
		switch (msg)
		{
		case WM_COMMAND:
		{
			auto notificationCode = HIWORD(wParam);
			if (notificationCode == BN_CLICKED)
			{
				auto* events = reinterpret_cast<const EventMap*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
				if (events)
				{
					auto controlId = static_cast<int>(LOWORD(wParam));
					if (auto* handler = events->findClickHandler(controlId))
					{
						auto event = ClickEvent{
							.controlId = controlId,
							.controlHwnd = reinterpret_cast<HWND>(lParam),
							.formHwnd = hwnd,
						};
						(*handler)(event);
						return 0;
					}
				}
			}
			return DefWindowProcW(hwnd, msg, wParam, lParam);
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProcW(hwnd, msg, wParam, lParam);
		}
	}

	// Creates and shows a top-level window from a Form definition.
	// Returns the HWND of the created window, or nullptr on failure.
	auto LoadForm(const Form& form, HINSTANCE hInstance, const EventMap& events) -> HWND
	{
		static constexpr auto ClassName = L"FormDesignerWindow";
		static auto registered = false;

		if (not registered)
		{
			auto wc = WNDCLASSEXW{
				.cbSize = sizeof(WNDCLASSEXW),
				.style = CS_HREDRAW | CS_VREDRAW,
				.lpfnWndProc = FormWndProc,
				.hInstance = hInstance,
				.hCursor = LoadCursorW(nullptr, IDC_ARROW),
				.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
				.lpszClassName = ClassName,
			};

			if (not RegisterClassExW(&wc))
			{
				std::println(std::cerr, "Failed to register form class: error {}", GetLastError());
				return nullptr;
			}
			registered = true;
		}

		// Calculate window rect so client area matches requested size.
		auto rc = RECT{ 0, 0, form.width, form.height };
		AdjustWindowRectEx(&rc, form.style, FALSE, form.exStyle);

		auto hwnd = CreateWindowExW(
			form.exStyle,
			ClassName,
			form.title.c_str(),
			form.style,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			rc.right - rc.left,
			rc.bottom - rc.top,
			nullptr,
			nullptr,
			hInstance,
			nullptr
		);

		if (not hwnd)
		{
			std::println(std::cerr, "Failed to create form window: error {}", GetLastError());
			return nullptr;
		}

		CreateChildren(hwnd, hInstance, form.controls);

		// Store event map pointer for WndProc dispatch.
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&events));

		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);

		return hwnd;
	}

	// Runs the message loop. Call after LoadForm.
	auto RunMessageLoop() -> int
	{
		auto msg = MSG{};
		while (GetMessageW(&msg, nullptr, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		return static_cast<int>(msg.wParam);
	}
}
