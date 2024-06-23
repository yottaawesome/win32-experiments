import std;
import samples;

int main()
{
    std::set_terminate([]() -> void {
        std::cerr << "terminate called after throwing an instance of ";
        try
        {
            std::rethrow_exception(std::current_exception());
        }
        catch (const std::exception& ex)
        {
            std::cerr << typeid(ex).name() << std::endl;
            std::cerr << "  what(): " << ex.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << typeid(std::current_exception()).name() << std::endl;
            std::cerr << " ...something, not an exception, dunno what." << std::endl;
        }
        std::abort();
        });

    try
    {
        int* x = nullptr;
        int y = *x;
    }
    catch (...)
    {
        int y = 10;
    }

    //return BasicWindow::Run();
    return Gradient::Run();
    //return ObjectOrientedControl::Run();
}
