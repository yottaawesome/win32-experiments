// Demo to dynamically create a dialog box with 
// multiple buttons using an in-memory dialog 
// template using CreateDialogIndirect.
// 
// Disclosure: created with Gemini 3 Pro's help.

#include <windows.h>
import std;

// Helper to align data on DWORD boundaries
// Dialog templates require DLGITEMTEMPLATEs to be DWORD aligned
void AlignToDword(std::vector<BYTE>& buffer)
{
    size_t size = buffer.size();
    if (size % 4 != 0)
    {
        size_t padding = 4 - (size % 4);
        for (size_t i = 0; i < padding; ++i)
            buffer.push_back(0);
    }
}

// Helper to append raw data to the buffer
template<typename T>
void AppendData(std::vector<BYTE>& buffer, const T& data)
{
    const BYTE* ptr = reinterpret_cast<const BYTE*>(&data);
    buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
}

// Helper to append a null-terminated Unicode string
template<>
void AppendData<std::wstring>(std::vector<BYTE>& buffer, const std::wstring& str)
{
    const BYTE* ptr = reinterpret_cast<const BYTE*>(str.c_str());
    buffer.insert(buffer.end(), ptr, ptr + (str.length() + 1) * sizeof(wchar_t));
}

// Dialog Procedure to handle button clicks
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
        case 1001: // Button 1
        case 1002: // Button 2
        case 1003: // Button 3
        case 1004: // Button 4
            {
                std::wstring msg = L"You clicked Button " + std::to_wstring(LOWORD(wParam) - 1000);
                MessageBoxW(hwndDlg, msg.c_str(), L"Clicked", MB_OK);
            }
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        return TRUE;
    }
    return FALSE;
}

int wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
    std::vector<BYTE> buffer;

    // 1. Construct the DLGTEMPLATE structure
    DLGTEMPLATE dlgTemplate = {};
    dlgTemplate.style = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_CENTER | DS_SETFONT;
    dlgTemplate.dwExtendedStyle = 0;
    dlgTemplate.cdit = 4; // Number of controls (4 buttons)
    dlgTemplate.x = 0;
    dlgTemplate.y = 0;
    dlgTemplate.cx = 200; // Width in dialog units
    dlgTemplate.cy = 150; // Height in dialog units

    AppendData(buffer, dlgTemplate);

    // Append Menu (0 = none), Class (0 = default), and Title
    AppendData<WORD>(buffer, 0); 
    AppendData<WORD>(buffer, 0); 
    AppendData<std::wstring>(buffer, L"Manual Dialog");

    // Append Font information (required because DS_SETFONT is used)
    AppendData<WORD>(buffer, 8); // Point size
    AppendData<std::wstring>(buffer, L"MS Shell Dlg");

    // 2. Construct DLGITEMTEMPLATE structures for 4 buttons
    for (int i = 0; i < 4; ++i)
    {
        // Each item must be DWORD aligned
        AlignToDword(buffer);

        DLGITEMTEMPLATE item = {};
        item.style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
        item.dwExtendedStyle = 0;
        item.x = 50;
        item.y = static_cast<short>(20 + (i * 25)); // Stack buttons vertically
        item.cx = 100;
        item.cy = 20;
        item.id = static_cast<WORD>(1001 + i);

        AppendData(buffer, item);

        // Class: 0xFFFF followed by 0x0080 indicates the system "Button" class
        AppendData<WORD>(buffer, 0xFFFF);
        AppendData<WORD>(buffer, 0x0080);

        // Title (Button Text)
        std::wstring btnText = L"Button " + std::to_wstring(i + 1);
        AppendData<std::wstring>(buffer, btnText);

        // Creation data (0 bytes)
        AppendData<WORD>(buffer, 0);
    }

    // 3. Display the dialog using the in-memory template
    DialogBoxIndirectParamW(hInstance, reinterpret_cast<LPCDLGTEMPLATE>(buffer.data()), nullptr, DialogProc, 0L);

    return 0;
}
