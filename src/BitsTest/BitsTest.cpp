#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <Windows.h>
#include <bits.h>
#include "common.h"
#include "ComHandler.h"
#include <memory>
#include <vector>
#include <exception>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
ComPtr<IBackgroundCopyManager> ptr = nullptr;

int main(int argc, char* args)
try
{
    ComHandler comInit;
    std::shared_ptr<IBackgroundCopyManager> bcm =
        comInit.CreateInstance<IBackgroundCopyManager, BackgroundCopyManager>();

    if(bcm != nullptr)
        std::cout << "Completed successfully\n";

    return 0;
}
catch (const std::runtime_error& ex)
{
    std::cout << ex.what() << std::endl;
    return 1;
}

