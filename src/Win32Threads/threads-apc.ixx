export module threads:apc;
import std;
import win32;
import :shared;

namespace
{
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

    template<typename F>
    struct IsPairT : std::false_type {};

    template<typename F, typename S>
    struct IsPairT<std::pair<F, S>> : std::true_type {};

    template<typename T>
    inline constexpr bool IsPairV = IsPairT<T>::value;

    template<typename T>
    concept Pair = IsPairV<T>;

    ///////////////////////
    struct TestType
    {
        TestType(int a, int b)
        {
            std::cout << std::format("TestType [1]: {} {}\n", a, b);
        }
        void operator()(int a, int b)
        {
            std::cout << std::format("TestType [2]: {} {}\n", a, b);
        }
    };

    template<typename...T>
    struct VariadicArgs
    {
        VariadicArgs(T&&...con)
            : Args{ std::forward<decltype(con)>(con)... } {}
        std::tuple<T...> Args;

        using TupleT = decltype(Args);
    };

    template<typename TInvokee, typename TConstructArgs, typename TInvokeArgs>
    struct ConstructInvoker
    {
        ConstructInvoker(TConstructArgs&& c, TInvokeArgs&& a)
            : ConstructorArgs{ std::move(c.Args) },
            InvokeArgs{ std::move(a.Args) }
        {}

        TConstructArgs::TupleT ConstructorArgs;
        TInvokeArgs::TupleT InvokeArgs;
        TInvokee Invokee = std::make_from_tuple<TInvokee>(ConstructorArgs);

        auto operator()() { return std::apply(Invokee, InvokeArgs); }
        auto operator()(auto&&...args) { return std::invoke(Invokee, std::forward<decltype(args)>(args)...); }
    };
    // Use like so
    // ConstructInvoker<TestType, VariadicArgs<int, int>, VariadicArgs<int, int>> l{ VariadicArgs{1, 1}, VariadicArgs{2, 2} };
    // l();
    ///////////////////////

    // https://devblogs.microsoft.com/oldnewthing/20200624-00/?p=103902
    template<typename Tuple, std::size_t... Ints>
    std::tuple<std::tuple_element_t<Ints, Tuple>...> SelectTuple(Tuple&& tuple, std::index_sequence<Ints...>)
    {
        return { std::get<Ints>(std::forward<Tuple>(tuple))... };
    }

    template<typename Tuple, std::size_t... Ints>
    std::tuple<std::tuple_element_t<Ints, Tuple>...> SelectTuple(Tuple& tuple, std::index_sequence<Ints...>)
    {
        return { std::get<Ints>(std::forward<Tuple>(tuple))... };
    }

    // https://devblogs.microsoft.com/oldnewthing/20200625-00/?p=103903
    template<std::size_t N, typename Seq>
    struct offset_sequence;

    template<std::size_t N, std::size_t... Ints>
    struct offset_sequence<N, std::index_sequence<Ints...>>
    {
        using type = std::index_sequence<Ints + N...>;
    };
    template<std::size_t N, typename Seq>
    using offset_sequence_t = typename offset_sequence<N, Seq>::type;

    template<typename...T>
    void QueueMemberFunctionTuple(std::tuple<T...>& args)
    {
        using TTuple = std::remove_cvref_t<decltype(args)>;
        constexpr size_t TupleSize = std::tuple_size_v<TTuple>;
        static_assert(sizeof...(T) >= 2, "2 or more args must be passed in.");

        auto& object = std::get<0>(args);
        auto& function = std::get<1>(args);

        if constexpr (TupleSize == 2)
        {
            std::invoke(function, object);
        }
        else if constexpr (TupleSize > 2)
        {
            auto argsTuple = SelectTuple(
                args,
                offset_sequence_t<2, std::make_index_sequence<TupleSize - 2>>{}
            );
            std::apply(function, std::tuple_cat(std::make_tuple(std::ref(object)), argsTuple));
        }
    }

    void QueueMemberFunctionTuple2(auto& object, auto function, auto&&...args)
    {
        std::invoke(function, object, std::forward<decltype(args)>(args)...);
    }

    struct Test
    {
        void Member(int)
        {
            std::println("Hello, indirect!");
        }
    };
}

export namespace APC
{
    SynchronizationBarrier<2> syncBarrier{};

	template<typename T, typename...TArgs>
    class Thread final
    {
        public:
            Thread() = default;
            Thread(TArgs&... args) : Args{ std::ref(args)... } {}
            Thread(const Thread&) = delete;
            Thread& operator=(const Thread&) = delete;
            Thread(Thread&&) = delete;
            Thread& operator=(Thread&&) = delete;
            std::tuple<TArgs&...> Args{};

            public:
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
                if (m_handle and win32::WaitForSingleObjectEx(m_handle.get(), win32::InfiniteWait, false))
                    throw system_category_error{ "WaitForSingleObjectEx() failed." };
            }

            auto Detach()               noexcept        -> HandleUniquePtr&&    { return std::move(m_handle); }
            auto GetHandle()            const noexcept  -> win32::HANDLE        { return m_handle.get(); }
            void PrintPointlessLine()                                           { std::println("Pointless line printed!"); }

            void Queue()
            {
                // https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-queueuserapc
                win32::DWORD success = 0;
                if constexpr (false)
                {
                    success = win32::QueueUserAPC(
                        ExecuteAPC<decltype(PrintPointlessLinePtr)>,
                        m_handle.get(),
                        reinterpret_cast<win32::ULONG_PTR>(&PrintPointlessLinePtr)
                    );
                }
                else
                {
                    success = win32::QueueUserAPC(
                        ExecuteAPC<decltype(PP)>,
                        m_handle.get(),
                        reinterpret_cast<win32::ULONG_PTR>(&PP)
                    );
                }
                if (not success)
                    throw system_category_error{ "QueueUserAPC() failed" };
            }

        private:
            // Use one or the other
            PtrToMember<Thread, &Thread::PrintPointlessLine> PrintPointlessLinePtr{ this };
            // Can also use decltype(&Thread::PrintPointlessLine)
            std::pair<Thread*, void(Thread::*)()> PP{this, &Thread::PrintPointlessLine };

            static unsigned __stdcall Run(void* ptr)
            {
                Thread* instance = reinterpret_cast<Thread*>(ptr);
                T task = std::make_from_tuple<T>(instance->Args);

                return task.Begin();
                /*return std::apply(
                    [&task](auto&&...args)
                    {
                        return task.Begin(std::forward<decltype(args)>(args)...);
                    },
                    instance->Args
                );*/
                // std::apply doesn't work here as as we need std::ref since Begin()
                // accepts a reference, but we can't do that easily with a tuple
                // and std::tuple_cat

                //return T::Begin(TArgs{}...);
            }

            template<typename T>
            static void ExecuteAPC(win32::ULONG_PTR ptr)
            {
                // Use one or the other
                if constexpr (Pair<T>)
                {
                    auto [obj, func] = *reinterpret_cast<T*>(ptr);
                    std::invoke(func, obj);

                    /*T p = *reinterpret_cast<T*>(ptr);
                    Thread* t = p.first;
                    auto fn = p.second;
                    (t->*(fn))();*/
                }
                else
                {
                    auto memberFuncPtr = reinterpret_cast<T*>(ptr);
                    (*memberFuncPtr)();
                }
            }

        private:
            HandleUniquePtr m_handle{};
            unsigned m_threadId = 0;
    };

    struct TaskState
    {
        int x = 0;
        bool Run = true;

        HandleUniquePtr Exit =
            [] {
                win32::HANDLE ptr = win32::CreateEventW(nullptr, true, false, nullptr);
                if (not ptr)
                    throw system_category_error("CreateEventW() failed.");
                return HandleUniquePtr(ptr);
            }();

        void SignalToExit()
        {
            win32::SetEvent(Exit.get());
        }
    };

    struct Task final
    {
        Task(TaskState& stateIn)
            : state{stateIn}
        {}

        TaskState& state;

        unsigned Begin()
        {
            syncBarrier.Enter();
            while (state.Run)
            {
                std::println("state value {}", state.x);
                win32::DWORD result = win32::WaitForSingleObjectEx(state.Exit.get(), win32::InfiniteWait, true);
                    
                //win32::SleepEx(win32::InfiniteWait, true);
                if (result == win32::WaitIoCompletion)
                    std::println("Matches");
                else
                    state.Run = false;
            }
            
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
        ts.SignalToExit();
        t.Join();
    }
}

export namespace APC2
{
    template<typename T>
    class Thread final
    {
        public:
            Thread(T& task) : Task{ task }{}
            Thread(const Thread&) = delete;
            Thread& operator=(const Thread&) = delete;
            Thread(Thread&&) = delete;
            Thread& operator=(Thread&&) = delete;

            T& Task;

        public:
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
                if (m_handle and win32::WaitForSingleObjectEx(m_handle.get(), win32::InfiniteWait, false))
                    throw system_category_error{ "WaitForSingleObjectEx() failed." };
            }

            auto Detach()               noexcept        -> HandleUniquePtr&& { return std::move(m_handle); }
            auto GetHandle()            const noexcept  -> win32::HANDLE { return m_handle.get(); }

            template<typename T, typename F, typename...TArgs>
            struct MemberFunc
            {
                MemberFunc(T* object, F f, auto&&...args)
                    : Object{ object }, Function{ f }, Args { std::forward<decltype(args)>(args)... }
                { }
                T* Object = nullptr;
                F Function;
                std::tuple<TArgs...> Args;
            };

            template<typename TObject, typename TFunction>
            void QueueMemberFunction(std::pair<TObject, TFunction>& objectAndMember)
            {
                win32::DWORD success = win32::QueueUserAPC(
                    ExecuteAPC<std::remove_cvref_t<decltype(objectAndMember)>>,
                    m_handle.get(),
                    reinterpret_cast<win32::ULONG_PTR>(&objectAndMember)
                );
                if (not success)
                    throw system_category_error{ "QueueUserAPC() failed" };
            }

            template<typename T, typename F, typename...A>
            void QueueMemberFunction2(T* object, F function, A&&...args)
            {
                auto m = new MemberFunc<T, F, A...>{ object, function, std::forward<A>(args)...};
                win32::DWORD success = win32::QueueUserAPC(
                    ExecuteAPC2<T, F, A...>,
                    m_handle.get(),
                    reinterpret_cast<win32::ULONG_PTR>(m)
                );
                if (not success)
                    throw system_category_error{ "QueueUserAPC() failed" };
            }

        private:
            // Use one or the other
            // Can also use decltype(&Thread::PrintPointlessLine)
            //std::pair<Thread*, void(Thread::*)()> PP{ this, &Thread::PrintPointlessLine };

            static unsigned __stdcall Run(void* ptr)
            {
                Thread* instance = reinterpret_cast<Thread*>(ptr);
                return instance->Task.Begin(instance);
            }

            template<typename T>
            static void ExecuteAPC(win32::ULONG_PTR ptr)
            {
                auto [obj, func] = *reinterpret_cast<T*>(ptr);
                std::invoke(func, obj);
            }

            template<typename T, typename F, typename...A>
            static void ExecuteAPC2(win32::ULONG_PTR ptr)
            {
                MemberFunc<T, F, A...>* info = reinterpret_cast<MemberFunc<T, F, A...>*>(ptr);
                if (not info)
                    return;
                std::unique_ptr<MemberFunc<T, F, A...>> uptr(info);
                std::apply(info->Function, std::tuple_cat(std::make_tuple(info->Object), info->Args));
            }

        private:
            HandleUniquePtr m_handle{};
            unsigned m_threadId = 0;
    };


    struct Task
    {
        void NoOp() { }

        void SomeFunc2(const int& i)
        {
            std::println("The value is {}", i);
        }

        std::pair<Task*, void(Task::*)()> PtrSomeFunc { this, &Task::NoOp };
        bool Run = true;

        void Exit()
        {
            Run = false;
            Thread->QueueMemberFunction2(this, &Task::NoOp);
        }

        unsigned Begin(Thread<Task>* thread)
        {
            Thread = thread;
            while(Run)
                win32::SleepEx(win32::InfiniteWait, true);

            return 0;
        }

        void Enqueue()
        {
            if (not Thread)
                return;
            Thread->QueueMemberFunction(PtrSomeFunc);
        }

        void Enqueue2()
        {
            if (not Thread)
                return;
            Thread->QueueMemberFunction2(this, &Task::SomeFunc2, 1);
        }

        Thread<Task>* Thread = nullptr;
    };

    void Run()
    {
        Task task{};
        Thread thread{ task };
        thread.Start();
        std::this_thread::sleep_for(std::chrono::seconds{ 2 });
        task.Enqueue2();
        task.Exit();
    }
}