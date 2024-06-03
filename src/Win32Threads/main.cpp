import std;
import win32;
import threads;

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
    //APC2::Run();
    ThreadMessageQueue::Run();

    return 0;
}
