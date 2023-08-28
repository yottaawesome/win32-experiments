// Adapted from https://stackoverflow.com/a/30523079/7448661

#include <windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <atomic>
#include "pdh.h"

#pragma comment(lib, "pdh.lib")

class Query 
{
    PDH_HQUERY cpuQuery;
    PDH_HCOUNTER cpuTotal;

    public:
        ~Query()
        {
            // https://learn.microsoft.com/en-us/windows/win32/api/pdh/nf-pdh-pdhremovecounter
            PdhRemoveCounter(cpuTotal);
            // https://learn.microsoft.com/en-us/windows/win32/api/pdh/nf-pdh-pdhclosequery
            PdhCloseQuery(cpuQuery);
        }

        Query() 
        {
            PdhOpenQueryW(nullptr, 0, &cpuQuery);
            PdhAddCounterW(cpuQuery, TEXT("\\Processor(_Total)\\% Processor Time"), 0, &cpuTotal);
            PdhCollectQueryData(cpuQuery);
        }

        operator double() 
        {
            PDH_FMT_COUNTERVALUE counterVal;

            PdhCollectQueryData(cpuQuery);
            PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, nullptr, &counterVal);
            return counterVal.doubleValue;
        }
};

int main() 
{
    Query q;
    Sleep(2000);
    std::cout << q << "\n";

    return 0;
}