#pragma once
#include <Windows.h>
#include <stdexcept>

template<typename T>
inline void ReleaseCOM(T** ptrCom)
{
    if (ptrCom != nullptr && *ptrCom != nullptr)
    {
        (*ptrCom)->Release();
        *ptrCom = nullptr;
    }
}

inline void CheckHR(HRESULT hr)
{
    if (FAILED(hr))
        throw std::runtime_error("Win32 function call failed");
}
