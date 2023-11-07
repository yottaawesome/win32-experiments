// Adapted from https://learn.microsoft.com/en-us/windows/win32/wmisdk/example--getting-wmi-data-from-the-local-computer
// See also https://learn.microsoft.com/en-us/windows/win32/wmisdk/wmi-c---application-examples
// And https://learn.microsoft.com/en-us/windows/win32/wmisdk/creating-a-wmi-application-using-c-
#define _WIN32_DCOM     // Not sure what this does.
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <iostream>
//import std;   // Fails to compile due to redefinitions;
                // the com headers must be bringing in 
                // standard headers somewhere. The culprit
                // appears to be comdef.h importing comip.h
                // which imports <type_traits>.

#pragma comment(lib, "wbemuuid.lib")

namespace GettingWMIDataFromLocalComputer
{
    int Run()
    {
        HRESULT hres;

        // Step 1: --------------------------------------------------
        // Initialize COM. ------------------------------------------

        hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres))
        {
            std::cout << "Failed to initialize COM library. Error code = 0x"
                << std::hex << hres << std::endl;
            return 1;                  // Program has failed.
        }

        // Step 2: --------------------------------------------------
        // Set general COM security levels --------------------------

        hres = CoInitializeSecurity(
            nullptr,
            -1,                          // COM authentication
            nullptr,                        // Authentication services
            nullptr,                        // Reserved
            RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
            RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
            nullptr,                        // Authentication info
            EOAC_NONE,                   // Additional capabilities 
            nullptr                         // Reserved
        );


        if (FAILED(hres))
        {
            std::cout << "Failed to initialize security. Error code = 0x"
                << std::hex << hres << std::endl;
            CoUninitialize();
            return 1;                    // Program has failed.
        }

        // Step 3: ---------------------------------------------------
        // Obtain the initial locator to WMI -------------------------

        IWbemLocator* pLoc = nullptr;

        hres = CoCreateInstance(
            CLSID_WbemLocator,
            0,
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, (LPVOID*)&pLoc);

        if (FAILED(hres))
        {
            std::cout << "Failed to create IWbemLocator object."
                << " Err code = 0x"
                << std::hex << hres << std::endl;
            CoUninitialize();
            return 1;                 // Program has failed.
        }

        // Step 4: -----------------------------------------------------
        // Connect to WMI through the IWbemLocator::ConnectServer method

        IWbemServices* pSvc = nullptr;

        // Connect to the root\cimv2 namespace with
        // the current user and obtain pointer pSvc
        // to make IWbemServices calls.
        hres = pLoc->ConnectServer(
            _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
            nullptr,                    // User name. NULL = current user
            nullptr,                    // User password. NULL = current
            0,                       // Locale. NULL indicates current
            0,                    // Security flags.
            0,                       // Authority (for example, Kerberos)
            0,                       // Context object 
            &pSvc                    // pointer to IWbemServices proxy
        );

        if (FAILED(hres))
        {
            std::cout << "Could not connect. Error code = 0x"
                << std::hex << hres << std::endl;
            pLoc->Release();
            CoUninitialize();
            return 1;                // Program has failed.
        }

        std::cout << "Connected to ROOT\\CIMV2 WMI namespace" << std::endl;


        // Step 5: --------------------------------------------------
        // Set security levels on the proxy -------------------------

        hres = CoSetProxyBlanket(
            pSvc,                        // Indicates the proxy to set
            RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
            RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
            nullptr,                        // Server principal name 
            RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
            RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
            nullptr,                        // client identity
            EOAC_NONE                    // proxy capabilities 
        );

        if (FAILED(hres))
        {
            std::cout << "Could not set proxy blanket. Error code = 0x"
                << std::hex << hres << std::endl;
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return 1;               // Program has failed.
        }

        // Step 6: --------------------------------------------------
        // Use the IWbemServices pointer to make requests of WMI ----

        // For example, get the name of the operating system
        IEnumWbemClassObject* pEnumerator = nullptr;
        hres = pSvc->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM Win32_OperatingSystem"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr,
            &pEnumerator);

        if (FAILED(hres))
        {
            std::cout << "Query for operating system name failed."
                << " Error code = 0x"
                << std::hex << hres << std::endl;
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return 1;               // Program has failed.
        }

        // Step 7: -------------------------------------------------
        // Get the data from the query in step 6 -------------------

        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;

        while (pEnumerator)
        {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
                &pclsObj, &uReturn);

            if (0 == uReturn)
            {
                break;
            }

            VARIANT vtProp;

            VariantInit(&vtProp);
            // Get the value of the Name property
            hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
            std::wcout << " OS Name : " << vtProp.bstrVal << std::endl;
            VariantClear(&vtProp);

            pclsObj->Release();
        }

        // Cleanup
        // ========

        pSvc->Release();
        pLoc->Release();
        pEnumerator->Release();
        CoUninitialize();

        return 0;   // Program successfully completed.
    }
}

int main(int argc, char** argv)
{
    return GettingWMIDataFromLocalComputer::Run();
}
