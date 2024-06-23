import std;

int main()
{
    // Adds about 100 kB versus dynamically-linking the runtime.
    try
    {
        throw std::runtime_error("hello world!");
    }
    catch (const std::exception& ex)
    {
        std::println("{}", ex.what());
    }

    return 0;
}
