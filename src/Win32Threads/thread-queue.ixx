export module threads:threadqueue;
import std;
import win32;

namespace Stuff
{
	void waitonall(std::same_as<win32::HANDLE> auto...handles)
	{
		((handles == nullptr ? (throw std::runtime_error("a"), true) : (false)), ...);
		([](win32::HANDLE h)
			{

			}(handles), ...);

		std::array v{ handles... };

	}

	void blah()
	{
		win32::HANDLE a = nullptr, b = nullptr;
		waitonall(a, b);
	}
}

// Demonstrates using a Win32 message queue in a different thread.
// See https://stackoverflow.com/questions/51942953/is-it-possible-to-create-a-message-loop-without-creating-a-window-in-c
export namespace ThreadMessageQueue
{
	// Some helper concepts.
	template<class T>
	struct IsDuration : std::false_type {};

	template<class Rep, class Period>
	struct IsDuration<std::chrono::duration<Rep, Period>> : std::true_type {};

	template<typename T>
	constexpr bool IsDurationV = IsDuration<T>::value;

	template<typename T>
	concept Duration = IsDuration<T>::value;

	// Used for cleaning up our event.
	struct HandleDeleter
	{
		void operator()(win32::HANDLE h)
		{
			win32::CloseHandle(h);
		}
	};
	using Win32HandleUniquePtr = std::unique_ptr<std::remove_pointer_t<win32::HANDLE>, HandleDeleter>;

	// A small event wrapper used to signal when the queue is ready.
	class Event
	{
		public:
		void Wait()
		{
			win32::DWORD result = win32::WaitForSingleObject(
				m_handle.get(),
				win32::InfiniteWait
			);
			if (result != win32::WaitResults::Index)
				throw std::runtime_error("WaitForSingleObject() failed.");
		}

		bool Wait(Duration auto&& duration)
		{
			win32::DWORD result = win32::WaitForSingleObject(
				m_handle.get(),
				static_cast<win32::DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count())
			);
			switch (result)
			{
				case win32::WaitResults::Index:
					return true;
				case win32::WaitResults::Timeout:
					return false;
				default:
					throw std::runtime_error("WaitForSingleObject() failed.");
			}
			std::unreachable();
		}

		void Signal()
		{
			win32::SetEvent(m_handle.get());
		}

		private:
		Win32HandleUniquePtr m_handle =
			[]{
				HANDLE handle = win32::CreateEventW(nullptr, true, false, nullptr);
				if (not handle)
					throw std::runtime_error("Failed creating handle");
				return Win32HandleUniquePtr(handle);
			}();
	};

	class BaseThread
	{
		public:
		virtual ~BaseThread() = default;

		void Start()
		{
			m_thread = std::jthread(
				[](BaseThread* self) noexcept -> int
				{
					try
					{
						self->m_threadId = win32::GetCurrentThreadId();
						self->Run();
						return 0;
					}
					catch (const std::exception& ex)
					{
						std::println("Thread encountered exception: {}", ex.what());
						return 1;
					}
				},
				this
			);
		}

		void WaitOnReady()
		{
			if (not m_ready.Wait(std::chrono::seconds{ 2 }))
				throw std::runtime_error("Timed out waiting for queue.");
		}

		virtual void PostMessage() = 0;
		virtual void PostQuit() = 0;

		protected:
		DWORD m_threadId = 0;
		Event m_ready;
		std::jthread m_thread;

		virtual void Run() = 0;
	};

	// Thread queue using a hidden window.
	class WindowThread : public BaseThread
	{
		public:
		void PostMessage() override
		{
			// synchronous, return value is based on the message
			//win32::SendMessageW(m_wnd, win32::Msg::MessageBase, 0, 0);
			// asynchronous
			if (not win32::PostMessageW(m_wnd, win32::Msg::MessageBase, 0, 0))
				throw std::runtime_error("PostMessageW() failed");
		}

		void PostQuit() override
		{
			// synchronous, return value is based on the message
			//win32::SendMessageW(m_wnd, win32::Msg::Close, 0, 0);
			// asynchronous
			if (not win32::PostMessageW(m_wnd, win32::Msg::Close, 0, 0))
				throw std::runtime_error("PostMessageW() failed");
			// This also works
			/*if (not win32::PostThreadMessageW(m_threadId, win32::Msg::QuitMsg, 0, 0))
				throw std::runtime_error("PostThreadMessageW() failed.");*/
		}

		private:
		win32::HWND m_wnd = nullptr;

		void Run() override
		{
			constexpr std::wstring_view name = L"TestWindowClass";
			constexpr std::wstring_view windowName = L"TestWindow";

			win32::WNDCLASSEXW windowClass{
				.cbSize = sizeof(win32::WNDCLASSEXW),
				.lpfnWndProc = WindowThread::WndProc,
				.hInstance = win32::GetModuleHandleW(nullptr),
				.lpszClassName = name.data(),
			};
			win32::ATOM atom = win32::RegisterClassExW(&windowClass);
			if (not atom)
				throw std::runtime_error("RegisterClassExW() failed");

			m_wnd = win32::CreateWindowExW(
				0, 
				name.data(), 
				windowName.data(), 
				0, 
				0, 
				0, 
				0, 
				0, 
				nullptr, 
				nullptr, 
				win32::GetModuleHandleW(nullptr), 
				nullptr
			);
			if (not m_wnd)
				throw std::runtime_error("Failed creating window");
			win32::ShowWindow(m_wnd, win32::Hide);

			m_ready.Signal();

			win32::MSG msg;
			while (true)
			{
				if (win32::BOOL result = win32::GetMessageW(&msg, nullptr, 0, 0); result <= 0)
				{
					std::println("GetMessage(): {}", result);
					break;
				}
				// win32::TranslateMessage(&msg); Not required
				win32::DispatchMessageW(&msg); // Not necessarily required, see not in WndProc.
				std::println("WindowThread success! {}", msg.message);
			}
			std::println("WindowThread bye!");
		}

		static win32::LRESULT WndProc(win32::HWND hwnd, win32::UINT32 msg, win32::WPARAM wparam, win32::LPARAM lparam)
		{
			// You probably don't need to process messages here. You'll need to spcifically handle
			// the WM_DWMNCRENDERINGCHANGED message if you don't.
			// https://learn.microsoft.com/en-us/windows/win32/dwm/wm-dwmncrenderingchanged
			switch (msg)
			{
				case win32::Msg::Close:
					std::println("Got Close!");
					win32::PostQuitMessage(0);
					return 0;
				case win32::Msg::MessageBase:
					std::println("Got MessageBase!");
					return 0;
			}
			return win32::DefWindowProcW(hwnd, msg, wparam, lparam);
		}
	};

	// Thread queue with no window.
	class NoWindowThread : public BaseThread
	{
		public:
		void PostMessage() override
		{
			if (not win32::PostThreadMessageW(m_threadId, win32::Msg::MessageBase, 0, 0))
				throw std::runtime_error("PostThreadMessageW() failed.");
		}

		void PostQuit() override
		{
			if (not win32::PostThreadMessageW(m_threadId, win32::Msg::QuitMsg, 0, 0))
				throw std::runtime_error("PostThreadMessageW() failed.");
		}

		private:
		void Run() override
		{
			win32::MSG msg;
			win32::PeekMessageW(&msg, nullptr, 0, 0, win32::NoRemove);
			m_ready.Signal();

			while (true)
			{
				if (win32::BOOL result = win32::GetMessageW(&msg, nullptr, 0, 0); result <= 0)
				{
					std::println("GetMessage(): {}", result);
					break;
				}
				std::println("NoWindowThread success! {}", msg.message);
			}
			std::println("NoWindowThread bye!");
		}		
	};

	auto Run() -> void
	try
	{
		std::println("Testing NoWindowThread...");
		NoWindowThread t;
		t.Start();
		t.WaitOnReady();
		t.PostMessage();
		t.PostQuit();

		std::this_thread::sleep_for(std::chrono::seconds{1});
		std::println("Testing NoWindowThread...");
		WindowThread w;
		w.Start();
		w.WaitOnReady();
		w.PostMessage();
		w.PostQuit();
	}
	catch (const std::exception& ex)
	{
		std::println("Run() failed: {}", ex.what());
	}
}
