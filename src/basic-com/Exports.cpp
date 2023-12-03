#include "AddObjFactory.hpp"
#include "AddObj.hpp"

HMODULE g_hModule;
constexpr auto AddObjProgId = L"CodeGuru.FastAddition";
long g_nComObjsInUse = 0;

STDAPI DllGetClassObject(
    const CLSID& clsid,
    const IID& iid,
    void** ppv
)
{
    //
    //Check if the requested COM object is implemented in this DLL
    //There can be more than 1 COM object implemented in a DLL
    //

    if (clsid == CAddObj::CLSID_AddObject)
    {
        //
        //iid specifies the requested interface for the factory object
        //The client can request for IUnknown, IClassFactory,
        //IClassFactory2
        //
        CAddFactory* pAddFact = new CAddFactory;
        if (pAddFact == NULL)
            return E_OUTOFMEMORY;

        return pAddFact->QueryInterface(iid, ppv);
    }


    //
    //if control reaches here then that implies that the object
    //specified by the user is not implemented in this DLL
    //

    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow()
{
    //A DLL is no longer in use when it is not managing any existing objects
    // (the reference count on all of its objects is 0). 
    //We will examine the value of g_nComObjsInUse 
    if (g_nComObjsInUse == 0)
    {
        return S_OK;
    }
    return S_FALSE;
}

BOOL   HelperWriteKey(
    HKEY roothk,
    const wchar_t* lpSubKey,
    LPCTSTR val_name,
    DWORD dwType,
    void* lpvData,
    DWORD dwDataSize)
{
    //
    //Helper function for doing the registry write operations
    //
    //roothk:either of HKCR, HKLM, etc

    //lpSubKey: the key relative to 'roothk'

    //val_name:the key value name where the data will be written

    //dwType:the type of data that will be written ,REG_SZ,REG_BINARY, etc.

    //lpvData:a pointer to the data buffer

    //dwDataSize:the size of the data pointed to by lpvData
    //
    //

    HKEY hk;
    if (ERROR_SUCCESS != RegCreateKey(roothk, lpSubKey, &hk)) return FALSE;

    if (ERROR_SUCCESS != RegSetValueEx(hk, val_name, 0, dwType, (CONST BYTE*)lpvData, dwDataSize)) return FALSE;

    if (ERROR_SUCCESS != RegCloseKey(hk))   return FALSE;
    return TRUE;

}



///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

extern "C" HRESULT __stdcall DllRegisterServer()
{
    //
    //As per COM guidelines, every self installable COM inprocess component
    //should export the function DllRegisterServer for printing the 
    //specified information to the registry
    //
    //

    WCHAR* lpwszClsid;
    wchar_t szBuff[MAX_PATH] = L"";
    wchar_t szClsid[MAX_PATH] = L"", szInproc[MAX_PATH] = L"", szProgId[MAX_PATH];
    wchar_t szDescriptionVal[256] = L"";

    StringFromCLSID(
        CAddObj::CLSID_AddObject,
        &lpwszClsid);

    wsprintf(szClsid, L"%S", lpwszClsid);
    wsprintf(szInproc, L"%s\\%s\\%s", "clsid", szClsid, "InprocServer32");
    wsprintf(szProgId, L"%s\\%s\\%s", "clsid", szClsid, "ProgId");


    //
    //write the default value 
    //
    wsprintf(szBuff, L"%s", "Fast Addition Algorithm");
    wsprintf(szDescriptionVal, L"%s\\%s", "clsid", szClsid);

    HelperWriteKey(
        HKEY_CLASSES_ROOT,
        szDescriptionVal,
        NULL,//write to the "default" value
        REG_SZ,
        (void*)szBuff,
        lstrlen(szBuff)
    );


    //
    //write the "InprocServer32" key data
    //
    GetModuleFileName(
        g_hModule,
        szBuff,
        sizeof(szBuff));
    HelperWriteKey(
        HKEY_CLASSES_ROOT,
        szInproc,
        NULL,//write to the "default" value
        REG_SZ,
        (void*)szBuff,
        lstrlen(szBuff)
    );

    //
    //write the "ProgId" key data under HKCR\clsid\{---}\ProgId
    //
    lstrcpy(szBuff, AddObjProgId);
    HelperWriteKey(
        HKEY_CLASSES_ROOT,
        szProgId,
        NULL,
        REG_SZ,
        (void*)szBuff,
        lstrlen(szBuff)
    );


    //
    //write the "ProgId" data under HKCR\CodeGuru.FastAddition
    //
    wsprintf(szBuff, L"%s", "Fast Addition Algorithm");
    HelperWriteKey(
        HKEY_CLASSES_ROOT,
        AddObjProgId,
        NULL,
        REG_SZ,
        (void*)szBuff,
        lstrlen(szBuff)
    );


    wsprintf(szProgId, L"%s\\%s", AddObjProgId, "CLSID");
    HelperWriteKey(
        HKEY_CLASSES_ROOT,
        szProgId,
        NULL,
        REG_SZ,
        (void*)szClsid,
        lstrlen(szClsid)
    );

    return 1;

}


///////////////////////////////////////////////////////////////////////////////

extern "C" HRESULT __stdcall DllUnregisterServer(void)
{
    //
    //As per COM guidelines, every self removable COM inprocess component
    //should export the function DllUnregisterServer for erasing all the 
    //information that was printed into the registry
    //
    //

    wchar_t szKeyName[256] = L"", szClsid[256] = L"";
    WCHAR* lpwszClsid;



    //
    //delete the ProgId entry
    //
    wsprintf(szKeyName, L"%s\\%s", AddObjProgId, "CLSID");
    RegDeleteKey(HKEY_CLASSES_ROOT, szKeyName);
    RegDeleteKey(HKEY_CLASSES_ROOT, AddObjProgId);


    //
    //delete the CLSID entry for this COM object
    //
    StringFromCLSID(
        CAddObj::CLSID_AddObject,
        &lpwszClsid);
    wsprintf(szClsid, L"%S", lpwszClsid);
    wsprintf(szKeyName, L"%s\\%s\\%s", "CLSID", szClsid, "InprocServer32");
    RegDeleteKey(HKEY_CLASSES_ROOT, szKeyName);

    wsprintf(szKeyName, L"%s\\%s\\%s", "CLSID", szClsid, "ProgId");
    RegDeleteKey(HKEY_CLASSES_ROOT, szKeyName);

    wsprintf(szKeyName, L"%s\\%s", "CLSID", szClsid);
    RegDeleteKey(HKEY_CLASSES_ROOT, szKeyName);

    return 1;

}