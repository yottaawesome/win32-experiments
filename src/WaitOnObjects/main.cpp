import std;
import win32;

namespace A
{
    template<typename T>
    concept Awaitable = requires(T t) { { t.GetHandle() } -> std::convertible_to<Win32::HANDLE>; };

    template<int VValue>
    using Integral = std::integral_constant<int, VValue>;

    template<typename...Ts>
    struct Overload : Ts...
    {
        using Ts::operator()...;
    };

    template<typename TVariant>
    struct Variant
    {
        TVariant Value;
        Variant(TVariant&& variant) : Value(std::forward<TVariant>(variant)) {}

        auto Visit(this auto&& self, auto&&...fn)
        {
            return std::visit(Overload{ std::forward<decltype(fn)>(fn)... }, std::forward_like<decltype(self)>(self.Value));
        }
    };

    auto WaitOn(Awaitable auto&&...awaitables)
    {
        static_assert(sizeof...(awaitables) > 0, "Must be greater than 0.");
        std::array handles{ awaitables.GetHandle()... };
        Win32::DWORD result = Win32::WaitForMultipleObjects(sizeof...(awaitables), handles.data(), false, Win32::Infinite);

        return []<size_t...Is>(Win32::DWORD result, std::index_sequence<Is...>) -> std::variant<Integral<Is>...>
        {
            std::variant<Integral<Is>...> returnValue;
            ((Integral<Is>{} == Is ? (returnValue = Integral<Is>{}, true) : false) or ...);
            return returnValue;
        }(result, std::make_index_sequence<sizeof...(awaitables)>{});
    }

    struct AwaitableType
    { 
        Win32::HANDLE Handle = nullptr;  
        auto GetHandle(this auto&& self) -> Win32::HANDLE { return self.Handle; }
    };

    void Run()
    {
        Win32::HANDLE eventA = Win32::CreateEventA(nullptr, false, false, nullptr);
        Win32::HANDLE eventB = Win32::CreateEventA(nullptr, false, false, nullptr);
        Variant{ 
            WaitOn(
                AwaitableType{eventA}, 
                AwaitableType{eventB}
            ) 
        }.Visit(
            [](Integral<0>)
            {
                
            },
            [](Integral<1>)
            {

            }
        );

        struct { Win32::HANDLE GetHandle() { return nullptr; } } X;
        WaitOn(X);
    }
}

namespace B
{
    template<typename...Ts>
    struct Overload : Ts...
    {
        using Ts::operator()...;
    };

    template<std::invocable TInvocable>
    struct AwaitableInvocable
    {
        Win32::HANDLE Handle = nullptr;
        TInvocable Invocable;

        AwaitableInvocable(Win32::HANDLE h, TInvocable&& invocable)
            : Handle(h), Invocable(std::forward<decltype(invocable)>(invocable))
        {}

        auto operator()(this auto&& self) { return std::invoke(self.Invocable); }
        auto GetHandle(this auto&& self) -> Win32::HANDLE { return self.Handle; }
    };

    using HandleUniquePtr = std::unique_ptr<std::remove_pointer_t<Win32::HANDLE>, decltype([](Win32::HANDLE h) { Win32::CloseHandle(h); })>;

    auto WaitOn(std::chrono::milliseconds wait, bool waitAll, bool alertable, auto...awaitables)
    {
        HandleUniquePtr dummy;
        ((awaitables.GetHandle() 
            ? false 
            : (dummy = HandleUniquePtr(Win32::CreateEventA(nullptr, false, false, nullptr)), true)
        ) or ...);

        static_assert(sizeof...(awaitables) > 0, "Must be greater than 0.");
        //((awaitables.GetHandle() == nullptr ? (throw std::runtime_error("Must not be null")) : false), ...);
        std::array handles{(awaitables.GetHandle() ? awaitables.GetHandle() : dummy.get())...};
        Win32::DWORD result = Win32::WaitForMultipleObjectsEx(
            sizeof...(awaitables), 
            handles.data(), 
            waitAll, 
            static_cast<Win32::DWORD>(wait.count()), 
            alertable
        );
        if (result == Win32::WaitFailed)
            throw std::runtime_error("The wait failed.");
        if (result >= Win32::WaitAbandoned0 and result < (Win32::WaitAbandoned0+sizeof...(awaitables)))
            throw std::runtime_error("The wait was abandoned.");

        //using VariantT = std::variant<std::invoke_result_t<decltype(awaitables)>...>;
        int index = 0;
        ([run = result == index++, awaitable = awaitables]()
        {
            return run ? (std::invoke(awaitable), true) : false;
        }() or ...);
    }

    void Run()
    {
        Win32::HANDLE eventA = Win32::CreateEventA(nullptr, false, false, nullptr);
        Win32::HANDLE eventB = Win32::CreateEventA(nullptr, false, false, nullptr);
        Win32::SetEvent(eventB);

        std::variant<int, int> S;
        std::visit([](int) { std::println("This is an integer."); }, S);

        WaitOn(
            std::chrono::milliseconds{Win32::Infinite},
            false, 
            false,
            AwaitableInvocable{ eventA, [] { std::println("A"); } }, 
            AwaitableInvocable{ eventB, [] { std::println("B"); } }
        );
    }

    template<typename T>
    struct IsPair : std::false_type {};
    template<std::invocable T>
    struct IsPair<std::pair<Win32::HANDLE, T>> : std::true_type {};
    template<typename T>
    constexpr bool IsPairV = IsPair<T>::value;
    template<typename T>
    concept AwaitablePair = IsPairV<T>;


    void AAA(auto&&...a)
    {

    }

    void Test()
    {
        AwaitablePair auto a = std::pair{ Win32::HANDLE{}, [] {} };
        AAA(std::make_pair(Win32::HANDLE{}, []{}));
    }
}

namespace C
{
    template<typename...Ts>
    struct Overload : Ts...
    {
        using Ts::operator()...;
    };

    template<typename T>
    struct Fired {};

    template<typename T>
    struct Awaitable
    {
        using Type = Awaitable<T>;
        Win32::HANDLE Handle = nullptr;
        auto GetHandle(this auto&& self) { return self.Handle; }
    };

    auto WaitOn(std::chrono::milliseconds wait, bool waitAll, bool alertable, auto&&...awaitables)
    {
        static_assert(sizeof...(awaitables) > 0, "Must be greater than 0.");

        //using VariantT = std::variant<typename std::remove_cvref_t<decltype(awaitables)>::Type...>; // 1, use with 2
        using VariantT = std::variant<typename std::remove_cvref_t<decltype(awaitables)>...>;

        std::array handles{ awaitables.GetHandle()... };
        Win32::DWORD result = Win32::WaitForMultipleObjectsEx(
            sizeof...(awaitables), 
            handles.data(), 
            waitAll, 
            static_cast<Win32::DWORD>(wait.count()), 
            alertable
        );
        if (result == Win32::WaitFailed)
            throw std::runtime_error("The wait failed.");
        if (result >= Win32::WaitAbandoned0 and result < (Win32::WaitAbandoned0 + sizeof...(awaitables)))
            throw std::runtime_error("The wait was abandoned.");

        VariantT returnValue;
        Win32::DWORD index = 0;
        ([&returnValue, result, index = index++, awaitable = awaitables] -> auto
        {
            return index == result 
                //? (returnValue = typename std::remove_cvref_t<decltype(awaitable)>::Type{}, true) // 2, use with 1
                ? (returnValue = awaitable, true)
                : false;
        }() or ...);
        return returnValue;
    }

    struct A {};

    template<typename TVariant>
    struct Variant
    {
        TVariant Value;
        Variant(TVariant&& variant) : Value(std::move(variant)) {}
        Variant(const TVariant& variant) : Value(variant) {}

        auto Visit(this auto&& self, auto&&...fn)
        {
            return std::visit(Overload{ std::forward<decltype(fn)>(fn)... }, std::forward_like<decltype(self)>(self.Value));
        }
    };

    void Run()
    {
        Win32::HANDLE eventA = Win32::CreateEventA(nullptr, false, false, nullptr);
        Win32::HANDLE eventB = Win32::CreateEventA(nullptr, false, false, nullptr);
        Win32::SetEvent(eventB);

        std::variant result = WaitOn(
            std::chrono::milliseconds{ Win32::Infinite },
            false,
            false,
            Awaitable<struct A>{eventA},
            Awaitable<struct B>{eventB}
        );
        Variant(result).Visit(
            [](Awaitable<struct A>) // Can also pass in the original Struct
            {
                std::println("A");
            },
            [](Awaitable<struct B>)
            {
                std::println("B");
            });
    }
}

int main()
{
    C::Run();
    return 0;
}
