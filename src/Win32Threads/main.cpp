import std;
import win32;
import threads;

int main()
{
    Threads1::Run();
    Threads2::Run();
    Group::Run();
    StdGroup::Run();
    APC::Run();
}
