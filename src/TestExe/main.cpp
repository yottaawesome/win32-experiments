import testdll;
import std;

int main()
{
    TestDLL::TestClass test;
    try
    {
        test.BB();
    }
    catch (const std::runtime_error&)
    {
        std::println("Success! {}", test.GetAString());
    }

    std::println("Hello World!");
}
