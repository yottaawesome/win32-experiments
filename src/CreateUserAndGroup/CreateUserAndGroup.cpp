#include <stdio.h>
#include <stdlib.h>
#include "Funcs.hpp"

int main()
{
    NET_API_STATUS err = 0;

    printf("Calling NetSample.\n");
    err = NetSampleNoDc(
        L"SampleUser",
        L"SamplePswd",
        L"SampleLG");
    printf("NetSample returned %d\n", err);
    DeleteLocalUserAndGroup(L"SampleUser", L"SampleLG");

    return 0;
}

