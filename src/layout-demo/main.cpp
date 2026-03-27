#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>
#include <algorithm>

// ============================================================
// Anchor Layout
// Each control remembers its initial position. On resize, the
// delta from the initial window size is applied based on which
// edges the control is anchored to.
// ============================================================

enum class Anchor : unsigned
{
    None   = 0,
    Left   = 1,
    Top    = 2,
    Right  = 4,
    Bottom = 8,
};

constexpr auto operator|(Anchor a, Anchor b) -> Anchor
{
    return static_cast<Anchor>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

constexpr auto HasFlag(Anchor value, Anchor flag) -> bool
{
    return (static_cast<unsigned>(value) & static_cast<unsigned>(flag)) != 0;
}

struct AnchorItem
{
    HWND hwnd;
    RECT initialRect;
    Anchor anchors;
};

struct AnchorLayout
{
    int initialWidth = 0;
    int initialHeight = 0;
    std::vector<AnchorItem> items;

    void Add(HWND hwnd, int x, int y, int w, int h, Anchor anchors)
    {
        items.push_back({ hwnd, { x, y, x + w, y + h }, anchors });
    }

    void Apply(int newWidth, int newHeight)
    {
        int dw = newWidth - initialWidth;
        int dh = newHeight - initialHeight;

        for (auto& item : items)
        {
            int x = item.initialRect.left;
            int y = item.initialRect.top;
            int w = item.initialRect.right - item.initialRect.left;
            int h = item.initialRect.bottom - item.initialRect.top;

            bool left   = HasFlag(item.anchors, Anchor::Left);
            bool top    = HasFlag(item.anchors, Anchor::Top);
            bool right  = HasFlag(item.anchors, Anchor::Right);
            bool bottom = HasFlag(item.anchors, Anchor::Bottom);

            if (left && right)        w += dw;
            else if (right && !left)  x += dw;

            if (top && bottom)        h += dh;
            else if (bottom && !top)  y += dh;

            w = (std::max)(w, 0);
            h = (std::max)(h, 0);
            MoveWindow(item.hwnd, x, y, w, h, TRUE);
        }
    }
};

// ============================================================
// Box Layout
// Controls are stacked vertically (VBox) or horizontally (HBox).
// Each item has a weight (proportional) or a fixed pixel size.
// ============================================================

struct LayoutItem
{
    HWND hwnd;
    float weight;
    int fixedSize;
};

struct VBox
{
    int spacing = 0;
    int margin = 0;
    std::vector<LayoutItem> items;

    void Apply(int x, int y, int w, int h)
    {
        if (items.empty()) return;

        float totalWeight = 0;
        int fixedTotal = margin * 2 + spacing * (static_cast<int>(items.size()) - 1);
        for (auto& item : items)
        {
            if (item.weight == 0.f) fixedTotal += item.fixedSize;
            else totalWeight += item.weight;
        }

        int flexible = (std::max)(0, h - fixedTotal);
        int cy = y + margin;

        for (auto& item : items)
        {
            int itemH = (item.weight > 0.f && totalWeight > 0.f)
                ? static_cast<int>(flexible * item.weight / totalWeight)
                : item.fixedSize;
            MoveWindow(item.hwnd, x + margin, cy, (std::max)(0, w - margin * 2), (std::max)(0, itemH), TRUE);
            cy += itemH + spacing;
        }
    }
};

// ============================================================
// Globals
// ============================================================

static AnchorLayout g_anchor;
static VBox g_vbox;

// ============================================================
// Control creation helpers
// ============================================================

static HWND CreateLabel(HWND parent, const wchar_t* text, int x, int y, int w, int h)
{
    return CreateWindowExW(
        0, L"STATIC", text,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, w, h,
        parent, nullptr, nullptr, nullptr);
}

static HWND CreateListbox(HWND parent, int x, int y, int w, int h)
{
    HWND lb = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
        x, y, w, h,
        parent, nullptr, nullptr, nullptr);

    for (int i = 1; i <= 20; i++)
    {
        wchar_t buf[64];
        wsprintfW(buf, L"Item %d", i);
        SendMessageW(lb, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(buf));
    }
    return lb;
}

static HWND CreatePushButton(HWND parent, const wchar_t* text, int x, int y, int w, int h)
{
    return CreateWindowExW(
        0, L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, y, w, h,
        parent, nullptr, nullptr, nullptr);
}

// ============================================================
// Window Procedures
// ============================================================

static LRESULT CALLBACK AnchorWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        g_anchor.Apply(LOWORD(lParam), HIWORD(lParam));
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK BoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        g_vbox.Apply(0, 0, LOWORD(lParam), HIWORD(lParam));
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ============================================================
// Entry Point
// ============================================================

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    // --- Register window classes ---

    WNDCLASSEXW wcAnchor{
        .cbSize = sizeof(WNDCLASSEXW),
        .lpfnWndProc = AnchorWndProc,
        .hInstance = hInstance,
        .hCursor = LoadCursorW(nullptr, IDC_ARROW),
        .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
        .lpszClassName = L"AnchorLayoutDemo",
    };
    RegisterClassExW(&wcAnchor);

    WNDCLASSEXW wcBox{
        .cbSize = sizeof(WNDCLASSEXW),
        .lpfnWndProc = BoxWndProc,
        .hInstance = hInstance,
        .hCursor = LoadCursorW(nullptr, IDC_ARROW),
        .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
        .lpszClassName = L"BoxLayoutDemo",
    };
    RegisterClassExW(&wcBox);

    // ========================================================
    // Anchor Layout Demo Window
    // Resize the window to see controls reposition:
    //   - Label stays at top-left
    //   - Listbox stretches to fill
    //   - Bottom-Right button sticks to the corner
    //   - Bottom-Left button sticks to the bottom edge
    // ========================================================

    HWND hAnchor = CreateWindowExW(
        0, L"AnchorLayoutDemo", L"Anchor Layout Demo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 520, 420,
        nullptr, nullptr, hInstance, nullptr);

    RECT rc;
    GetClientRect(hAnchor, &rc);
    g_anchor.initialWidth = rc.right;
    g_anchor.initialHeight = rc.bottom;

    int cw = rc.right;
    int ch = rc.bottom;

    g_anchor.Add(
        CreateLabel(hAnchor, L"Anchored: Top|Left (stays put)", 10, 10, 250, 20),
        10, 10, 250, 20,
        Anchor::Top | Anchor::Left);

    g_anchor.Add(
        CreateListbox(hAnchor, 10, 35, cw - 20, ch - 85),
        10, 35, cw - 20, ch - 85,
        Anchor::Top | Anchor::Left | Anchor::Right | Anchor::Bottom);

    g_anchor.Add(
        CreatePushButton(hAnchor, L"Bottom-Right", cw - 120, ch - 40, 110, 30),
        cw - 120, ch - 40, 110, 30,
        Anchor::Bottom | Anchor::Right);

    g_anchor.Add(
        CreatePushButton(hAnchor, L"Bottom-Left", 10, ch - 40, 110, 30),
        10, ch - 40, 110, 30,
        Anchor::Bottom | Anchor::Left);

    // ========================================================
    // Box Layout Demo Window
    // Resize the window to see the VBox redistribute space:
    //   - Label and buttons stay at fixed heights
    //   - Listbox stretches to fill remaining space
    // ========================================================

    HWND hBox = CreateWindowExW(
        0, L"BoxLayoutDemo", L"Box Layout Demo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 520, 420,
        nullptr, nullptr, hInstance, nullptr);

    GetClientRect(hBox, &rc);

    HWND boxLabel   = CreateLabel(hBox, L"Box Layout \u2014 controls stack vertically", 0, 0, 100, 20);
    HWND boxList    = CreateListbox(hBox, 0, 0, 100, 100);
    HWND boxButton1 = CreatePushButton(hBox, L"Fixed Button 1", 0, 0, 100, 30);
    HWND boxButton2 = CreatePushButton(hBox, L"Fixed Button 2", 0, 0, 100, 30);

    g_vbox.margin = 10;
    g_vbox.spacing = 5;
    g_vbox.items = {
        { boxLabel,   0.f, 20 },
        { boxList,    1.f, 0  },
        { boxButton1, 0.f, 30 },
        { boxButton2, 0.f, 30 },
    };
    g_vbox.Apply(0, 0, rc.right, rc.bottom);

    // --- Show both windows ---

    ShowWindow(hAnchor, nCmdShow);
    UpdateWindow(hAnchor);
    ShowWindow(hBox, nCmdShow);
    UpdateWindow(hBox);

    // --- Message loop ---

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
