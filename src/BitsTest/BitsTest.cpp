#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <Windows.h>
#include <bits.h>
#include "common.h"
#include "ComInitializer.h"

int main(int argc, char* args) try 
{
    IBackgroundCopyManager* g_pbcm = nullptr;
    ComInitializer comInit;

    CheckHR(
        CoCreateInstance(
            __uuidof(BackgroundCopyManager),
            nullptr,
            CLSCTX_LOCAL_SERVER,
            __uuidof(IBackgroundCopyManager),
            (void**)&g_pbcm
        )
    );

    ReleaseCOM(&g_pbcm);
    if(g_pbcm == nullptr)
        std::cout << "Completed successfully\n";

    return 0;
}
catch (const std::runtime_error& ex) {
    std::cout << ex.what() << std::endl;
    return 1;
}

