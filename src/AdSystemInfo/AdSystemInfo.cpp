#include <activeds.h>
#include <stdio.h>
#include <iostream>
#include <Iads.h>

#pragma comment(lib,"adsiid.lib")

int main()
{
    HRESULT hr;

    hr = CoInitialize(NULL);

    IADsADSystemInfo* pSys;
    hr = CoCreateInstance(CLSID_ADSystemInfo,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IADsADSystemInfo,
        (void**)&pSys);

    if (FAILED(hr))
        throw std::runtime_error("Failed to initialize IADsADSystemInfo");

    BSTR bstr;
    hr = pSys->get_UserName(&bstr);
    if (SUCCEEDED(hr)) {
        printf("User: %S\n", bstr);
        SysFreeString(bstr);
    }
    else
    {
        std::cout << "Failed" << std::endl;
    }

    hr = pSys->get_ComputerName(&bstr);
    if (SUCCEEDED(hr)) {
        printf("Computer: %S\n", bstr);
        SysFreeString(bstr);
    }

    hr = pSys->get_DomainDNSName(&bstr);
    if (SUCCEEDED(hr)) {
        printf("Domain: %S\n", bstr);
        SysFreeString(bstr);
    }

    hr = pSys->get_PDCRoleOwner(&bstr);
    if (SUCCEEDED(hr)) {
        printf("PDC Role owner: %S\n", bstr);
        SysFreeString(bstr);
    }

    if (pSys) {
        pSys->Release();
    }

    CoUninitialize();
    return 0;
}