export module CpuId:UI.Control;
import std;
import Win32;

export namespace UI
{
    template<typename TImplementation>
    class Control
    {
    protected:
        static auto SubclassProc(
            Win32::HWND hwnd,
            unsigned uMsg,
            Win32::WPARAM wParam,
            Win32::LPARAM lParam,
            Win32::UINT_PTR,
            Win32::DWORD_PTR dwRefData
        ) -> Win32::LRESULT
        {
            if (auto pThis = reinterpret_cast<TImplementation*>(dwRefData); pThis)
                if (auto result = pThis->HandleMessage(hwnd, uMsg, wParam, lParam); result)
                    return *result;
            return Win32::DefSubclassProc(hwnd, uMsg, wParam, lParam);
        }
    };
}
