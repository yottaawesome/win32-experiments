export module SspiExample:Common;
import std;

export namespace SspiExample
{
    void MyHandleError(const char* s)
    {
        std::println("{} error. Exiting", s);
        std::abort();
    }
}