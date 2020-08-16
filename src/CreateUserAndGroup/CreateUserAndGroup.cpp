#include <iostream>
#include "Funcs.hpp"

int main()
{
    std::wcout << "Calling NetSample." << std::endl;
    NET_API_STATUS err = NetSampleNoDc(
        L"SampleUser",
        L"SamplePswd",
        L"SampleLG");
    std::wcout << "NetSample returned: " << err << std::endl;
    LogonLocalUser(L"SampleUser", L"SamplePswd");
    DeleteLocalUserAndGroup(L"SampleUser", L"SampleLG");

    return 0;
}
