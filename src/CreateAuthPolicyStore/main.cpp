// Adapted from https://learn.microsoft.com/en-us/windows/win32/secauthz/creating-an-authorization-policy-store-object-in-c--
#include <windows.h>
#include <azroles.h>
#include <objbase.h>

import std;

void MyHandleError(std::string_view s)
{
    std::println("An error occurred in running the program.");
    std::println("{}", s);
    std::println("Error number {}.", GetLastError());
    std::println("Program terminating.");
    std::exit(1);
}

void WithActiveDirector() 
{
    IAzAuthorizationStore* pStore = NULL;
    HRESULT hr;
    BSTR storeName = NULL;

    //  Initialize COM.
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (!(SUCCEEDED(hr)))
        MyHandleError("Could not initialize COM.");

    //  Create the AzAuthorizationStore object.
    hr = CoCreateInstance(
        /*"b2bcff59-a757-4b0b-a1bc-ea69981da69e"*/
        __uuidof(AzAuthorizationStore),
        NULL,
        CLSCTX_ALL,
        /*"edbd9ca9-9b82-4f6a-9e8b-98301e450f14"*/
        __uuidof(IAzAuthorizationStore),
        (void**)&pStore);
    if (!(SUCCEEDED(hr)))
        MyHandleError("Could not create AzAuthorizationStore object.");

    //  Create a null VARIANT for function parameters.
    VARIANT myVar;
    VariantInit(&myVar);

    //  Allocate a string for the distinguished name of the
 //  Active Directory store.
    if (!(storeName = SysAllocString
    (L"msldap://CN=MyAzStore,CN=Program Data,DC=authmanager,DC=com")))
        MyHandleError("Could not allocate string.");

    //  Initialize the store in Active Directory. Use the
 //  AZ_AZSTORE_FLAG_CREATE flag.
    hr = pStore->Initialize(AZ_AZSTORE_FLAG_CREATE, storeName, myVar);
    if (!(SUCCEEDED(hr)))
        MyHandleError("Could not initialize store.");

    //  Call the submit method to save changes to the new store.
    hr = pStore->Submit(0, myVar);
    if (!(SUCCEEDED(hr)))
        MyHandleError("Could not save data to the store.");

    //  Clean up resources.
    pStore->Release();
    VariantClear(&myVar);
    SysFreeString(storeName);
    CoUninitialize();
}

void WithMSSQL() 
{
    IAzAuthorizationStore* pStore = NULL;
    HRESULT hr;
    BSTR storeName = NULL;

    //  Initialize COM.
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (!(SUCCEEDED(hr)))
        MyHandleError("Could not initialize COM.");

    //  Create the AzAuthorizationStore object.
    hr = CoCreateInstance(
        /*"b2bcff59-a757-4b0b-a1bc-ea69981da69e"*/
        __uuidof(AzAuthorizationStore),
        NULL,
        CLSCTX_ALL,
        /*"edbd9ca9-9b82-4f6a-9e8b-98301e450f14"*/
        __uuidof(IAzAuthorizationStore),
        (void**)&pStore);
    if (!(SUCCEEDED(hr)))
        MyHandleError("Could not create AzAuthorizationStore object.");

    VARIANT myVar;
    myVar.vt = VT_NULL;

    //  Allocate a string for the SQL Server store.
    if (!(storeName = SysAllocString
    (L"MSSQL://Driver={SQL Server};Server={AzServer};/AzDB/MyStore")))
        MyHandleError("Could not allocate string.");

    //  Initialize the store. Use the
 //  AZ_AZSTORE_FLAG_CREATE flag.
    hr = pStore->Initialize(AZ_AZSTORE_FLAG_CREATE, storeName, myVar);
    if (!(SUCCEEDED(hr)))
        MyHandleError("Could not initialize store.");

    //  Call the submit method to save changes to the new store.
    hr = pStore->Submit(0, myVar);
    if (!(SUCCEEDED(hr)))
        MyHandleError("Could not save data to the store.");

    //  Clean up resources.
    pStore->Release();
    SysFreeString(storeName);
    CoUninitialize();
}

void WithXML()
{
    IAzAuthorizationStore* pStore = nullptr;
    HRESULT hr;
    BSTR storeName = nullptr;

    //  Initialize COM.
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (!(SUCCEEDED(hr)))
        MyHandleError("Could not initialize COM.");

    //  Create the AzAuthorizationStore object.
    hr = CoCreateInstance(
        /*"b2bcff59-a757-4b0b-a1bc-ea69981da69e"*/
        __uuidof(AzAuthorizationStore),
        nullptr,
        CLSCTX_ALL,
        /*"edbd9ca9-9b82-4f6a-9e8b-98301e450f14"*/
        __uuidof(IAzAuthorizationStore),
        (void**)&pStore);
    if (!(SUCCEEDED(hr)))
        MyHandleError("Could not create AzAuthorizationStore object.");

    VARIANT myVar;
    myVar.vt = VT_NULL;

    //  Allocate a string for the distinguished name of the XML store.
    if (!(storeName = SysAllocString(L"msxml://C:\\MyStore.xml")))
        MyHandleError("Could not allocate string.");

    //  Initialize the store in an XML file. Use the
 //  AZ_AZSTORE_FLAG_CREATE flag.
    hr = pStore->Initialize(AZ_AZSTORE_FLAG_CREATE, storeName, myVar);
    if (!(SUCCEEDED(hr)))
        MyHandleError("Could not initialize store.");

    //  Call the submit method to save changes to the new store.
    hr = pStore->Submit(0, myVar);
    if (!(SUCCEEDED(hr)))
        MyHandleError("Could not save data to the store.");

    //  Clean up resources.
    pStore->Release();
    SysFreeString(storeName);
    CoUninitialize();
}

int main()
{
    return 0;
}