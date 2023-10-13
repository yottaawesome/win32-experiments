// Based on https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-wakebyaddresssingle
#include <windows.h>
import std;

#pragma comment(lib, "Synchronization.lib")

int main()
{
    unsigned long g_targetValue = 0; // global, accessible to all threads
    unsigned long undesiredValue = 0;
    unsigned long capturedValue = g_targetValue;

    std::jthread t(
        [&]{
            std::this_thread::sleep_for(std::chrono::seconds(1));
            g_targetValue = 1;
            // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-wakebyaddresssingle
            // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-wakebyaddressall
            WakeByAddressSingle(&g_targetValue);
        }
    );

    while (capturedValue == undesiredValue) 
    {
        // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitonaddress
        WaitOnAddress(&g_targetValue, &undesiredValue, sizeof(unsigned long), INFINITE);
        capturedValue = g_targetValue;
    }

    std::cout << std::format("Finished -- target value {}...\n", g_targetValue);

    return 0;
}
