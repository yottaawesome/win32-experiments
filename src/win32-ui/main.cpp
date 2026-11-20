#pragma comment(lib, "Comctl32.lib")
// See https://stackoverflow.com/questions/4308503/how-to-enable-visual-styles-without-a-manifest
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")

import std;
import samples;

void TestException()
{
    std::set_terminate(
        []() -> void
        {
            try
            {
                if (auto ex = std::current_exception(); ex)
                {
                    std::cerr << "terminate called after throwing an instance of ";
                    std::rethrow_exception(ex);
                }
                std::abort();
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
}

int main()
{
	return LatestSample::Run();
    //return BasicWindow::Run();
    //return Gradient::Run();
    //return ObjectOrientedControl::Run();
    //ChildControls::Run();
    return 0;
}
