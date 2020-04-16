#include <random>
#include <iostream>

int main()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(2000, 6000); // distribution in range [1, 6]

    std::cout << dist6(rng) << std::endl;

    return 0;
}

