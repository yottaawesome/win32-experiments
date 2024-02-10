#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <process.h>
import std;

struct HandleDeleter { void operator()(HANDLE h) { CloseHandle(h); } };
using HandleUniquePtr = std::unique_ptr<std::remove_pointer_t<HANDLE>, HandleDeleter>;

namespace Threads1
{
    template<typename T, typename...TArgs>
    class Thread final
    {
        public:
            ~Thread() { Join(); }
            Thread() = default;
            Thread(const Thread&) = delete;
            Thread& operator=(const Thread&) = delete;
            Thread(Thread&&) = delete;
            Thread& operator=(Thread&&) = delete;

            void Start()
            {
                const uintptr_t handle = _beginthreadex(
                    nullptr,
                    0,
                    Run,
                    nullptr,
                    0,
                    &m_threadId
                );
                m_handle = HandleUniquePtr(reinterpret_cast<HANDLE>(handle));
                if (not m_handle)
                    throw std::system_error(
                        std::error_code{ static_cast<int>(GetLastError()), std::system_category() }, 
                        "_beginthreadex() failed."
                    );
            }

            void Join() const
            {
                if (WaitForSingleObjectEx(m_handle.get(), INFINITE, false))
                    throw std::system_error(
                        std::error_code{ static_cast<int>(GetLastError()), std::system_category() }, 
                        "WaitForSingleObjectEx() failed."
                    );
            }
        
        private:
            static unsigned __stdcall Run(void* ptr)
            {
                T t{};
                return t.Begin(TArgs{}...);
            }

        private:
            HandleUniquePtr m_handle{};
            unsigned m_threadId = 0;
    };

    struct Alternative {};

    struct Runner
    {
        unsigned Begin()
        {
            std::println("Hello, Win32 threads!");
            return 0;
        }
        unsigned Begin(Alternative)
        {
            std::println("Hello, world and Win32 threads!");
            return 0;
        }
    };

    void Run()
    {
        Thread<Runner, Alternative> t;
        t.Start();
    }
}

namespace Threads2
{
    template<typename T, typename...TArgs>
    class Thread final
    {
        public:
            Thread() = default;
            Thread(const Thread&) = delete;
            Thread& operator=(const Thread&) = delete;
            Thread(Thread&&) = delete;
            Thread& operator=(Thread&&) = delete;

            void Start()
            {
                const uintptr_t handle = _beginthreadex(
                    nullptr,
                    0,
                    Run,
                    nullptr,
                    0,
                    &m_threadId
                );
                m_handle = HandleUniquePtr(reinterpret_cast<HANDLE>(handle));
                if (not m_handle)
                    throw std::system_error(
                        std::error_code{ static_cast<int>(GetLastError()), std::system_category() },
                        "_beginthreadex() failed."
                    );
            }

            void Join() const
            {
                if (WaitForSingleObjectEx(m_handle.get(), INFINITE, false))
                    throw std::system_error(
                        std::error_code{ static_cast<int>(GetLastError()), std::system_category() },
                        "WaitForSingleObjectEx() failed."
                    );
            }

            HandleUniquePtr&& Detach() noexcept
            {
                return std::move(m_handle);
            }

        private:
            static unsigned __stdcall Run(void* ptr)
            {
                return T::Begin(TArgs{}...);
            }

        private:
            HandleUniquePtr m_handle{};
            unsigned m_threadId = 0;
    };

    template<typename...T>
    void Run(std::vector<HandleUniquePtr>& handles)
    {
        (
            []<typename T, size_t...Is>(std::vector<HandleUniquePtr>& handles, Thread<T>&& t, std::index_sequence<Is...>)
            {
                t.Start();
                handles.emplace_back(t.Detach());
            }(handles, Thread<T>{}, std::make_index_sequence<sizeof...(T)>{})
        , ...);
    }

    struct Runner
    {
        static unsigned Begin()
        {
            std::println("Hello, Win32 threads!");
            return 0;
        }
    };

    void Run()
    {
        std::vector<HandleUniquePtr> v;
        Run<Runner, Runner, Runner>(v);
        for (auto& h : v)
            WaitForSingleObject(h.get(), INFINITE);
    }
}

namespace Group
{
    template<typename...T>
    struct ThreadGroup
    {
        std::tuple<T...> Threads{};

        void Start()
        {
            std::apply([](auto&...t) { (t.Start(), ...); }, Threads);
        }

        void Join()
        {
            std::apply([](auto&...t) { (t.Join(), ...); }, Threads);
        }
    };

    void Run()
    {
        ThreadGroup<Threads1::Thread<Threads1::Runner>> tg;
        tg.Start();
    }
}

namespace StdGroup
{
    struct Task final
    { 
        void operator()() { std::println("Hello, std::thread!"); } 
    };

    template<typename...T>
    struct ThreadGroup
    {
        std::array<std::jthread, sizeof...(T)> Threads;
        
        void Start()
        {
            Threads = { std::jthread(T{})... };
        }
    };

    void Run()
    {
        ThreadGroup<Task, Task, Task> ts{};
        ts.Start();
    }
}

int main()
{
    Threads1::Run();
    Threads2::Run();
    Group::Run();
    StdGroup::Run();
}
