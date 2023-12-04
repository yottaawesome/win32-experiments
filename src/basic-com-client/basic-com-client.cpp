#include <windows.h>
#include <iostream>
#include "../basic-com/IAdd_h.h"

int main()
{
    CoInitializeEx(
        nullptr, 
        COINIT::COINIT_APARTMENTTHREADED
        // | COINIT::COINIT_MULTITHREADED // Cannot be multithreaded
        | COINIT::COINIT_DISABLE_OLE1DDE
    );
    static constexpr GUID CLSID_AddObject =
        { 0x92e7a9c2, 0xf4cb, 0x11d4, { 0x82, 0x5d, 0x0, 0x10, 0x4b, 0x36, 0x46, 0xc0 } };

    static constexpr GUID IID_AddObject =
        { 0x1221db62, 0xf3d8, 0x11d4, {0x82, 0x5d, 0x00, 0x10, 0x4b, 0x36, 0x46, 0xc0 } };

    IAdd* v = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_AddObject, nullptr, CLSCTX_INPROC_SERVER, IID_AddObject, reinterpret_cast<void**>(&v));
    hr = v->SetFirstNumber(1);
    hr = v->SetSecondNumber(2);
    long result = 0;
    hr = v->DoTheAddition(&result);
    hr = v->Release();
    std::cout << result << std::endl;

    return 0;
}
