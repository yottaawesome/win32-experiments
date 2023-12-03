#include "AddObj.hpp"
#include "AddObjFactory.hpp"

HRESULT __stdcall CAddFactory::CreateInstance(IUnknown* pUnknownOuter,
    const IID& iid,
    void** ppv)
{
    //This method lets the client manufacture components en masse
    //The class factory provides a mechanism to control the way
    //the component is created. Within the class factory the 
    //author of the component may decide to selectivey enable
    //or disable creation as per license agreements 

    // Cannot aggregate.
    if (pUnknownOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    // Create an instance of the component.
    CAddObj* pObject = new CAddObj;
    if (pObject == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Get the requested interface.
    return pObject->QueryInterface(iid, ppv);
}


HRESULT __stdcall CAddFactory::LockServer(BOOL bLock)
{
    return E_NOTIMPL;
}

HRESULT __stdcall CAddFactory::QueryInterface(
    REFIID riid,
    void** ppObj
)
{
    if (riid == IID_IUnknown)
    {
        *ppObj = this;
        AddRef();
        return S_OK;
    }

    if (riid == IID_IAdd)
    {
        *ppObj = this;
        AddRef();
        return S_OK;
    }

    //if control reaches here then , let the client know that
    //we do not satisfy the required interface
    *ppObj = NULL;
    return E_NOINTERFACE;
}//QueryInterface method

ULONG __stdcall CAddFactory::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG __stdcall CAddFactory::Release()
{
    long nRefCount = 0;
    nRefCount = InterlockedDecrement(&m_nRefCount);
    if (nRefCount == 0)
        delete this;
    return nRefCount;
}