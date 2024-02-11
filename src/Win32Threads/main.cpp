import std;
import win32;
import threads;

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

int main()
{
    /*Test t;
    std::tuple arg = std::make_tuple(std::ref(t), &Test::Member, 1);
    QueueMemberFunctionTuple(arg);
    QueueMemberFunctionTuple2(t, &Test::Member, 1);*/

    /*Threads1::Run();
    Threads2::Run();
    Group::Run();
    StdGroup::Run();
    APC::Run();*/
    APC2::Run();

    return 0;
}
