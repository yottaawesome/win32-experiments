export module threads:threadqueue;
import std;
import win32;

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

	// Use for cleaning up our event.
	struct HandleDeleter
	{
		void operator()(win32::HANDLE h)
		{
			win32::CloseHandle(h);
		}
	};
	using Win32HandleUniquePtr = std::unique_ptr<std::remove_pointer_t<win32::HANDLE>, HandleDeleter>;

	// A small event wrapper to use to signal when the queue is ready.
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

	// The thread creating the message queue and receives messages on it.
	class Thread
	{
		public:
			void Start()
			{
				m_thread = std::jthread(
					[](Thread* self) 
					{
						self->m_threadId = win32::GetCurrentThreadId();
						self->RunNoWindow();
						return 0;
					}, 
					this
				);
			}

			void WaitOnReady()
			{
				if (not m_ready.Wait(std::chrono::seconds{ 2 }))
					throw std::runtime_error("Timed out waiting for queue.");
			}

			void PostMessage()
			{
				if (not win32::PostThreadMessageW(m_threadId, win32::MessageBase, 0, 0))
					throw std::runtime_error("PostThreadMessageW() failed.");
			}

			void PostQuit()
			{
				if (not win32::PostThreadMessageW(m_threadId, win32::QuitMsg, 0, 0))
					throw std::runtime_error("PostThreadMessageW() failed.");
			}

		private:
			DWORD m_threadId = 0;
			Event m_ready;
			std::jthread m_thread;

			void RunNoWindow()
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
					std::println("Success!");
				}
				std::println("Bye!");
			}

			void RunHiddenWindow()
			{
				// TODO
			}			
	};

	auto Run() -> void 
	try
	{
		Thread t;
		t.Start();
		t.WaitOnReady();
		t.PostMessage();
		t.PostQuit();
	}
	catch (const std::exception& ex)
	{
		std::println("Run() failed: {}", ex.what());
	}
}
