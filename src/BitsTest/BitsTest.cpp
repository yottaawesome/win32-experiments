#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <Windows.h>
#include <bits.h>
#include "common.h"
#include "ComHandler.h"
#include <memory>
#include <vector>

void test(std::vector<int>& v)
{
    v.push_back(1);
}

int main(int argc, char* args)
try
{
    byte* buffer = new byte[11];
    ComHandler comInit;
    std::shared_ptr<IBackgroundCopyManager> bcm =
        comInit.CreateInstance<BackgroundCopyManager, IBackgroundCopyManager>();

    if(bcm != nullptr)
        std::cout << "Completed successfully\n";

    return 0;
}
catch (const std::runtime_error& ex)
{
    std::cout << ex.what() << std::endl;
    return 1;
}

