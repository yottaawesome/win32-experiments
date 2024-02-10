export module threads:apc;
import std;
import win32;
import :shared;

export namespace APC
{
    SynchronizationBarrier<2> syncBarrier{};

    template<typename T, auto M>
    struct PtrToMember
    {
        T* t = nullptr;
        auto operator()(auto&&...args)
        {
            return std::invoke(M, t, std::forward<decltype(args)>(args)...);

            //(t->*(M))();
            //T& a = *t;
            //(t.*(M))();
        }
    };

	template<typename T, typename...TArgs>
    class Thread final
    {
        public:
            Thread() = default;
            Thread(TArgs&... args) : Args{ args... } {}
            Thread(const Thread&) = delete;
            Thread& operator=(const Thread&) = delete;
            Thread(Thread&&) = delete;
            Thread& operator=(Thread&&) = delete;
            std::tuple<TArgs...> Args{};

            void Start()
            {
                const win32::uintptr_t handle = win32::_beginthreadex(
                    nullptr,
                    0,
                    Run,
                    this,
                    0,
                    &m_threadId
                );
                m_handle = HandleUniquePtr(reinterpret_cast<win32::HANDLE>(handle));
                if (not m_handle)
                    throw system_category_error{ "_beginthreadex() failed." };
            }

            void Join() const
            {
                if (win32::WaitForSingleObjectEx(m_handle.get(), win32::InfiniteWait, false))
                    throw system_category_error{ "WaitForSingleObjectEx() failed." };
            }

            HandleUniquePtr&& Detach() noexcept
            {
                return std::move(m_handle);
            }

            win32::HANDLE GetHandle() const noexcept
            {
                return m_handle.get();
            }

            template<typename T>
            static void Something(win32::ULONG_PTR ptr) 
            {
                auto funcPtr = reinterpret_cast<T*>(ptr);
                (*funcPtr)();
            }

            void Ha() { std::println("Printed"); }
            PtrToMember<Thread, &Thread::Ha> HaPtr{ this };

            void Queue()
            {
                // https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-queueuserapc
                win32::DWORD success = win32::QueueUserAPC(
                    Something<decltype(HaPtr)>, 
                    m_handle.get(), 
                    reinterpret_cast<win32::ULONG_PTR>(&HaPtr)
                );
                if (not success)
                    throw system_category_error{ "QueueUserAPC() failed" };
            }

        private:
            static unsigned __stdcall Run(void* ptr)
            {
                Thread* instance = reinterpret_cast<Thread*>(ptr);
                return std::apply(T::Begin, instance->Args);
                //return T::Begin(TArgs{}...);
            }

        private:
            HandleUniquePtr m_handle{};
            unsigned m_threadId = 0;
    };

    struct TaskState
    {
        int x = 0;
    };

    struct Task
    {
        static unsigned Begin(TaskState& state)
        {
            syncBarrier.Enter();
            std::println("state value {}", state.x);
            win32::DWORD result = win32::SleepEx(win32::InfiniteWait, true);
            if (result == win32::WaitIoCompletion)
                std::println("Matches");
            return 0;
        }
    };

    void Run()
    {
        TaskState ts{ 10 };
        Thread<Task, TaskState> t{ ts };
        t.Start();
        syncBarrier.Enter();
        t.Queue();
        t.Join();
    }
}