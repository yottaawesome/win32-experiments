#include <iostream>

int wmain(int argc, wchar_t** args)
{
    for(int i = 0; i < argc; i++)
        std::wcout << args[i] << std::endl;

    std::wcout << "Hello World!" << std::endl;

    return 0;
}
