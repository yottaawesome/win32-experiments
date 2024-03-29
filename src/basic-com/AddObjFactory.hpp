#pragma once
#include <combaseapi.h>
///////////////////////////////////////////////////////////
//
//AddObjFactory.h
//Contains the C++ class declarations for the IClassFactory implementations
//
class CAddFactory : public IClassFactory
{
    public:
        //interface IUnknown methods 
        HRESULT __stdcall QueryInterface(
            REFIID riid,
            void** ppObj);
        ULONG   __stdcall AddRef();
        ULONG   __stdcall Release();


        //interface IClassFactory methods 
        HRESULT __stdcall CreateInstance(IUnknown* pUnknownOuter,
            const IID& iid,
            void** ppv);
        HRESULT __stdcall LockServer(BOOL bLock);

    private:
        long m_nRefCount;
};