#pragma once

///////////////////////////////////////////////////////////
//
//AddObj.h
//Contains the C++ class declarations for implementing the IAdd
//interfaces
//
#include    "IAdd_h.h"
extern long g_nComObjsInUse;
class CAddObj : public IAdd
{
    public:
    //IUnknown interface 
        HRESULT __stdcall QueryInterface(
            REFIID riid,
            void** ppObj
        );
        ULONG   __stdcall AddRef();
        ULONG   __stdcall Release();

        //IAdd interface
        HRESULT __stdcall SetFirstNumber(long nX1);
        HRESULT __stdcall SetSecondNumber(long nX2);
        HRESULT __stdcall DoTheAddition(long* pBuffer);

    public:
        // {92E7A9C2-F4CB-11d4-825D-00104B3646C0}
        static constexpr GUID CLSID_AddObject =
        { 0x92e7a9c2, 0xf4cb, 0x11d4, { 0x82, 0x5d, 0x0, 0x10, 0x4b, 0x36, 0x46, 0xc0 } };

    private:
        long m_nX1 = 0;
        long m_nX2 = 0; //operands for addition
        long m_nRefCount = 0;   //for managing the reference count
};

///////////////////////////////////////////////////////////