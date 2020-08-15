#include <iostream>
#include <chrono>
#include <Windows.h>

int main(int argc, char** args)
{
    const auto p1 = std::chrono::system_clock::now();

    std::cout << "seconds since epoch: "
        << std::chrono::duration_cast<std::chrono::seconds>(
            p1.time_since_epoch()).count() << '\n';

    return 0;
}
