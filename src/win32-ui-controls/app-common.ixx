export module app:common;
import std;
import :win32;

export namespace Raii
{
	template<auto VDeleteFn>
	struct Deleter
	{
		static void operator()(auto resource) noexcept { VDeleteFn(resource); }
	};
	template<typename TResource, auto VDeleteFn>
	using UniquePtr = std::unique_ptr<TResource, Deleter<VDeleteFn>>;

	template<typename TResource, auto VDeleteFn>
	using DirectUniquePtr = std::unique_ptr<TResource, Deleter<VDeleteFn>>;
	template<typename TResource, auto VDeleteFn>
	using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<TResource>, Deleter<VDeleteFn>>;

	using LocalHeapUniquePtr = IndirectUniquePtr<Win32::HLOCAL, Win32::LocalFree>;
}

export namespace Error
{
	struct SystemMessage
	{
		SystemMessage(Win32::DWORD code) {}

		static auto Format(Win32::DWORD errorCode) noexcept -> std::string
		{
			void* messageBuffer = nullptr;

			constexpr auto Flags =
				Win32::FormatMessageOptions::AllocateBuffer
				| Win32::FormatMessageOptions::FromSystem
				| Win32::FormatMessageOptions::IgnoreInserts;
			Win32::FormatMessageA(
				Flags,
				nullptr, //module
				errorCode,
				0,
				reinterpret_cast<char*>(&messageBuffer),
				0,
				nullptr
			);
			if (not messageBuffer)
			{
				auto lastError = Win32::GetLastError();
				return std::format("Failed formatting code {} with error {}.", errorCode, lastError);
			}
			Raii::LocalHeapUniquePtr ptr(messageBuffer);
			std::string_view msg(static_cast<char*>(messageBuffer));
			// This should never happen
			// See also https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-raisefailfastexception
			if (Win32::LocalFree(messageBuffer))
				Win32::__fastfail(Win32::FailFast::Fatal);

			return msg | std::ranges::views::filter([](auto c) { return c != '\n' and c != '\r'; }) | std::ranges::to<std::string>();
		}

		std::string Message;
	};

	struct Error : std::exception
	{
		Error(std::string_view error) : std::exception(error.data()) {}
	};

	struct Win32Error : Error
	{
		Win32Error(Win32::DWORD errorCode = Win32::GetLastError(), std::source_location loc = std::source_location::current())
			: Error(Format(errorCode, "An error occurred", loc))
		{
		}

		Win32Error(Win32::DWORD errorCode, std::string_view message, std::source_location loc = std::source_location::current())
			: Error(Format(errorCode, message, loc))
		{
		}

		auto Format(
			this auto&& self,
			Win32::DWORD errorCode,
			std::string_view message,
			const std::source_location& loc
		) -> std::string
		{
			return std::format("{}: {}", errorCode, SystemMessage::Format(errorCode));
		}
	};
}


export namespace UI
{
	using HwndUniquePtr = Raii::IndirectUniquePtr<Win32::HWND, Win32::DestroyWindow>;
	using FontUniquePtr = Raii::IndirectUniquePtr<Win32::HFONT, Win32::DeleteObject>;

	struct CreateWindowArgs
	{
		Win32::PCWSTR WindowName = nullptr;
		Win32::DWORD Style = 0;
		Win32::DWORD ExtendedStyle = 0;
		int X = Win32::CwUseDefault;
		int Y = Win32::CwUseDefault;
		int Width = Win32::CwUseDefault;
		int Height = Win32::CwUseDefault;
		Win32::HWND ParentWindow = 0;
		Win32::HMENU Menu = 0;
	};

	//
	// Strongly-typed message.
	template<Win32::DWORD VMsg>
	struct Win32Message
	{
		static constexpr std::uint32_t uMsg = VMsg;
		Win32::HWND Hwnd = nullptr;
		Win32::WPARAM wParam = 0;
		Win32::LPARAM lParam = 0;
		constexpr auto operator==(this auto&&, std::uint32_t msg) noexcept -> bool
		{
			return VMsg == msg;
		}
	};

	template<typename T, typename M>
	concept Handles = requires(T t, M m)
	{
		{ t.OnMessage(m) } -> std::convertible_to<Win32::LRESULT>;
	};

	struct GenericWin32Message
	{
		Win32::HWND Hwnd = nullptr;
		Win32::UINT uMsg = 0;
		Win32::WPARAM wParam = 0;
		Win32::LPARAM lParam = 0;
	};

	struct Window
	{
		auto GetHandle(this auto&& self) noexcept { return self.m_window.get(); }
		void Destroy(this auto&& self) noexcept { self.m_window.reset(); }
		void SetFont(this auto&& self, Win32::HFONT font)
		{
			if (self.m_window)
				Win32::SendMessageW(self.m_window.get(), Win32::Messages::SetFont, reinterpret_cast<Win32::WPARAM>(font), true);
		}
		//
		// Shows or hide the window, if present.
		auto Show(this auto&& self) noexcept -> decltype(auto)
		{
			if (self.m_window)
				Win32::ShowWindow(self.m_window.get(), Win32::ShowWindowOptions::ShowNormal);
			return std::forward<decltype(self)>(self);
		}

		auto Hide(this auto&& self) noexcept -> decltype(auto)
		{
			if (self.m_window)
				Win32::ShowWindow(self.m_window.get(), Win32::ShowWindowOptions::Hide);
			return std::forward<decltype(self)>(self);
		}

		auto TakeFocus(this auto&& self) -> decltype(auto)
		{
			if (self.m_window)
				Win32::SetFocus(self.m_window.get());
			return std::forward<decltype(self)>(self);
		}
	protected:
		HwndUniquePtr m_window = nullptr;
	};

	template<unsigned VVirtualKeyCode>
	struct KeyBindable
	{
		constexpr auto KeyCode(this auto&&) noexcept -> unsigned { return VVirtualKeyCode; }
	};

	struct Textable
	{
		auto GetText(this const auto& self) -> std::wstring
		{
			auto handle = self.GetHandle();
			if (not handle)
				return {};
			std::wstring buffer(Win32::GetWindowTextLengthW(handle) + 1, '\0');
			Win32::GetWindowTextW(self.GetHandle(), buffer.data(), static_cast<int>(buffer.size()));
			// Remove trailing null character.
			return buffer.empty() ? buffer : (buffer.pop_back(), buffer);
		}

		auto SetText(this auto&& self, std::wstring_view text)
		{
			// Beware of embedded nulls -- these will cut the displayed text short.
			if (auto handle = self.GetHandle())
				Win32::SetWindowTextW(handle, text.data());
		}

		auto AppendText(this auto&& self, std::wstring_view text)
		{
			self.SetText(std::format(L"{}{}", self.GetText(), text));
		}

		auto ClearText(this auto&& self)
		{
			self.SetText(L"");
		}
	};

	template<bool VRedraw>
	struct MouseTracking
	{
		bool MouseHovering = false;

		auto OnMessage(this auto&& self, Win32Message<Win32::Messages::MouseHover> msg) -> Win32::LRESULT
		{
			self.MouseHovering = true;
			if constexpr (VRedraw)
			{
				Win32::RECT rc;
				Win32::GetClientRect(self.GetHandle(), &rc);
				Win32::InvalidateRect(self.GetHandle(), nullptr, false);
			}
			return 0;
		}

		auto OnMessage(this auto&& self, Win32Message<Win32::Messages::MouseLeave> msg) -> Win32::LRESULT
		{
			self.MouseHovering = false;
			if constexpr (VRedraw)
			{
				Win32::RECT rc;
				Win32::GetClientRect(self.GetHandle(), &rc);
				Win32::InvalidateRect(self.GetHandle(), nullptr, false);
			}
			return 0;
		}

		auto OnMessage(this auto&& self, Win32Message<Win32::Messages::MouseMove> msg) -> Win32::LRESULT
		{
			Win32::TRACKMOUSEEVENT tme{
				.cbSize = sizeof(tme),
				.dwFlags = Win32::TrackMouseEvents::Hover | Win32::TrackMouseEvents::Leave,
				.hwndTrack = self.GetHandle(),
				.dwHoverTime = 25 //0.1s
			};
			Win32::TrackMouseEvent(&tme);
			return 0;
		}

		auto IsMouseInWindow(this auto&& self) -> bool
		{
			Win32::DWORD msgpos = Win32::GetMessagePos();
			Win32::POINT pt = { Win32::GetXParam(msgpos), Win32::GetYParam(msgpos) };
			Win32::ScreenToClient(self.GetHandle(), &pt);
			Win32::RECT cr;
			Win32::GetClientRect(self.GetHandle(), &cr);
			return Win32::PtInRect(&cr, pt);
		}
	};
}
