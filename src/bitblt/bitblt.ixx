// Adapted from https://learn.microsoft.com/en-us/windows/win32/gdi/capturing-an-image
module;

#include <windows.h>

export module bitblt;
import std;

// RAII helpers
template<auto VDeleter>
struct Deleter
{
    static auto operator()(auto ptr) { VDeleter(ptr); }
};
template<typename T, auto VDeleter>
using DirectUniquePtr = std::unique_ptr<T, Deleter<VDeleter>>;
template<typename T, auto VDeleter>
using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<T>, Deleter<VDeleter>>;

struct DcUniquePtr
{
    ~DcUniquePtr()
    {
        Close();
    }

    DcUniquePtr() = default;
    DcUniquePtr(HWND hwnd, HDC dc)
        : Hwnd(hwnd), Dc(dc) 
    { }

    DcUniquePtr(const DcUniquePtr&) = delete;
    DcUniquePtr& operator=(const DcUniquePtr) = delete;

    DcUniquePtr(DcUniquePtr&& other)
    {
        Move(other);
    }

    DcUniquePtr& operator=(DcUniquePtr&& other)
    {
        Move(other);
        return *this;
    }

    void Move(DcUniquePtr& other)
    {
        Close();
        Hwnd = other.Hwnd;
        Dc = other.Dc;
        other.Hwnd = nullptr;
        other.Dc = nullptr;
    }

    void Close()
    {
        if (Dc)
        {
            ReleaseDC(Hwnd, Dc);
            Hwnd = nullptr;
            Dc = nullptr;
        }
    }

    HWND Hwnd = nullptr;
    HDC Dc = nullptr;
};

template<typename T>
using ObjectUniquePtr = IndirectUniquePtr<T, DeleteObject>;
using HandleUniquePtr = IndirectUniquePtr<HANDLE, CloseHandle>;
using CompatibleDcUniquePtr = IndirectUniquePtr<HDC, DeleteDC>;
template<typename T>
using LocalHeapUniquePtr = IndirectUniquePtr<T, LocalFree>;

// Global Variables:
HINSTANCE hInst;
constexpr std::wstring_view WindowTitle = L"someapp";
constexpr std::wstring_view WindowClass = L"somewindow";

auto CaptureAnImage(HWND hWnd) -> int 
{
    // Retrieve the handle to a display device context for the client 
    // area of the window. 
    DcUniquePtr hdcScreen{ nullptr, GetDC(NULL) };
    DcUniquePtr hdcWindow{ hWnd, GetDC(hWnd) };

    // Create a compatible DC, which is used in a BitBlt from the window DC.
    CompatibleDcUniquePtr hdcMemDC{ CreateCompatibleDC(hdcWindow.Dc) };
    if (not hdcMemDC)
    {
        MessageBox(hWnd, L"CreateCompatibleDC has failed", L"Failed", MB_OK);
        std::abort();
    }

    // Get the client area for size calculation.
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

    // This is the best stretch mode.
    SetStretchBltMode(hdcWindow.Dc, HALFTONE);

    // The source DC is the entire screen, and the destination DC is the current window (HWND).
    BOOL success = StretchBlt(
        hdcWindow.Dc,
        0, 0,
        rcClient.right,
        rcClient.bottom,
        hdcScreen.Dc,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        SRCCOPY
    );
    if (not success)
    {
        MessageBox(hWnd, L"StretchBlt has failed", L"Failed", MB_OK);
        std::abort();
    }

    // Create a compatible bitmap from the Window DC.
    ObjectUniquePtr<HBITMAP> hbmScreen{
        CreateCompatibleBitmap(hdcWindow.Dc, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top)
    };
    if (not hbmScreen)
    {
        MessageBox(hWnd, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);
        std::abort();
    }

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(hdcMemDC.get(), hbmScreen.get());
    // Bit block transfer into our compatible memory DC.
    success = BitBlt(
        hdcMemDC.get(),
        0, 0,
        rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
        hdcWindow.Dc,
        0, 0,
        SRCCOPY
    );
    if (not success)
    {
        MessageBox(hWnd, L"BitBlt has failed", L"Failed", MB_OK);
        std::abort();
    }

    // Get the BITMAP from the HBITMAP.
    BITMAP bmpScreen;
    GetObject(hbmScreen.get(), sizeof(BITMAP), &bmpScreen);

    BITMAPINFOHEADER bi{
        .biSize = sizeof(BITMAPINFOHEADER),
        .biWidth = bmpScreen.bmWidth,
        .biHeight = bmpScreen.bmHeight,
        .biPlanes = 1,
        .biBitCount = 32,
        .biCompression = BI_RGB,
        .biSizeImage = 0,
        .biXPelsPerMeter = 0,
        .biYPelsPerMeter = 0,
        .biClrUsed = 0,
        .biClrImportant = 0,
    };

    DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    // SAMPLE NOTE
    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
    // have greater overhead than HeapAlloc.
    // /SAMPLE NOTE
    // Converted to HeapAlloc, which has lower overhead and is already fixed with no locking required.
    LocalHeapUniquePtr<char*> lpbitmap{ (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBmpSize) };

    // Gets the "bits" from the bitmap, and copies them into a buffer 
    // that's pointed to by lpbitmap.
    GetDIBits(
        hdcWindow.Dc, 
        hbmScreen.get(), 
        0,
        (UINT)bmpScreen.bmHeight,
        lpbitmap.get(),
        (BITMAPINFO*)&bi, 
        DIB_RGB_COLORS
    );

    // A file is created, this is where we will save the screen capture.
    HANDLE hFile = CreateFile(
        L"captureqwsx.bmp",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, 
        nullptr
    );
    if (not hFile or hFile == INVALID_HANDLE_VALUE)
        std::abort();
    HandleUniquePtr file{ hFile };

    DWORD dwBytesWritten = 0;
    BITMAPFILEHEADER bmfHeader{
        .bfType = 0x4D42, // bfType must always be BM for Bitmaps..
        // Add the size of the headers to the size of the bitmap to get the total file size.
        .bfSize = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER),
        // Offset to where the actual bitmap bits start.
        .bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER)
    };
    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, nullptr);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, nullptr);
    WriteFile(hFile, (LPSTR)lpbitmap.get(), dwBmpSize, &dwBytesWritten, nullptr);

    return 0;
}

auto WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    switch (message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            CaptureAnImage(hWnd);
            EndPaint(hWnd, &ps);
        }
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    return 0;
}

auto MyRegisterClass(HINSTANCE hInstance) -> ATOM
{
    WNDCLASSEXW wcex{
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WndProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = hInstance,
        .hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION)),
        .hCursor = LoadCursor(nullptr, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName = nullptr,
        .lpszClassName = WindowClass.data(),
        .hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION))
    };
    return RegisterClassExW(&wcex);
}

void InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable
    HWND hWnd = CreateWindowW(WindowClass.data(), WindowTitle.data(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd)
        throw std::runtime_error("CreateWindowW() failed.");
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
}

extern "C" int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    MyRegisterClass(hInstance);
    InitInstance(hInstance, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
