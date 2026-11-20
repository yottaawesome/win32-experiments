export module Samples:Gradient;
import std;
import Shared;

namespace Gradient
{
    // https://stackoverflow.com/questions/18745447/how-can-i-change-the-background-color-of-a-button-winapi-c
    constexpr auto IDC_EXIT_BUTTON = 101;
    constexpr auto IDC_PUSHLIKE_BUTTON = 102;

    auto CreateGradientBrush(Win32::COLORREF top, Win32::COLORREF bottom, Win32::LPNMCUSTOMDRAW item) -> Win32::HBRUSH
    {
        auto Brush = Win32::HBRUSH{ nullptr };
        auto hdcmem = Win32::CreateCompatibleDC(item->hdc);
        auto hbitmap = Win32::CreateCompatibleBitmap(item->hdc, item->rc.right - item->rc.left, item->rc.bottom - item->rc.top);
        Win32::SelectObject(hdcmem, hbitmap);

        auto r1 = int{ Win32::GetRedValue(top) };
        auto r2 = int{ Win32::GetRedValue(bottom) };
        auto g1 = int{ Win32::GetGreenValue(top) };
        auto g2 = int{ Win32::GetGreenValue(bottom) };
        auto b1 = int{ Win32::GetBlueValue(top) };
        auto b2 = int{ Win32::GetBlueValue(bottom) };
        for (auto i = 0; i < item->rc.bottom - item->rc.top; i++)
        {
            auto temp = Win32::RECT{};
            auto r = int(r1 + double(i * (r2 - r1) / item->rc.bottom - item->rc.top));
            auto g = int(g1 + double(i * (g2 - g1) / item->rc.bottom - item->rc.top));
            auto b = int(b1 + double(i * (b2 - b1) / item->rc.bottom - item->rc.top));
            Brush = Win32::CreateSolidBrush(Win32::GetRGB(r, g, b));
            temp.left = 0;
            temp.top = i;
            temp.right = item->rc.right - item->rc.left;
            temp.bottom = i + 1;
            Win32::FillRect(hdcmem, &temp, Brush);
            Win32::DeleteObject(Brush);
        }
        auto pattern = Win32::CreatePatternBrush(hbitmap);

        Win32::DeleteDC(hdcmem);
        Win32::DeleteObject(Brush);
        Win32::DeleteObject(hbitmap);

        return pattern;
    }

    void CreateButtons(Win32::HWND hwnd)
    {
        auto Exit_Button = Win32::CreateWindowExW(
            0,
            L"BUTTON",
            L"EXIT",
            Win32::Styles::Visible | Win32::Styles::Child | Win32::Styles::PushButton,
            50,
            50,
            100,
            100,
            hwnd,
            reinterpret_cast<Win32::HMENU>(IDC_EXIT_BUTTON),
            nullptr,
            nullptr
        );
        if (Exit_Button == nullptr)
        {
            Win32::MessageBoxW(nullptr, L"Button Creation Failed!", L"Error!", Win32::MessageBoxes::IconExclamation);
            std::terminate();
        }

        auto Pushlike_Button = Win32::CreateWindowExW(
            0,
            L"BUTTON",
            L"PUSH ME!",
            Win32::Styles::Visible | Win32::Styles::Child | Win32::Styles::AutoCheckBox | Win32::Styles::PushLike,
            200,
            50,
            100,
            100,
            hwnd,
            (Win32::HMENU)IDC_PUSHLIKE_BUTTON,
            nullptr,
            nullptr
        );
        if (Pushlike_Button == nullptr)
        {
            Win32::MessageBoxW(nullptr, L"Button Creation Failed!", L"Error!", Win32::MessageBoxes::IconExclamation);
            std::terminate();
        }
    }

    auto __stdcall MainWindow(Win32::HWND hwnd, Win32::UINT msg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> Win32::LRESULT
    {
        static auto defaultbrush = Win32::HBRUSH{ nullptr };
        static auto hotbrush = Win32::HBRUSH{ nullptr };
        static auto selectbrush = Win32::HBRUSH{ nullptr };
        static auto push_uncheckedbrush = Win32::HBRUSH{ nullptr };
        static auto push_checkedbrush = Win32::HBRUSH{ nullptr };
        static auto push_hotbrush1 = Win32::HBRUSH{ nullptr };
        static auto push_hotbrush2 = Win32::HBRUSH{ nullptr };

        switch (msg)
        {
            case Win32::Messages::Create:
            {
                CreateButtons(hwnd);
                break;
            }

            case Win32::Messages::Command:
            {
                switch (Win32::GetLowWord(wParam))
                {
                    case IDC_EXIT_BUTTON:
                    {
                        Win32::SendMessageW(hwnd, Win32::Messages::Close, 0, 0);
                    }
                    break;
                }
                break;
            }

            case Win32::Messages::CtlColorBtn: //In order to make those edges invisble when we use RoundRect(),
            {                //we make the color of our button's background match window's background
                return (Win32::LRESULT)Win32::GetSysColorBrush(Win32::ColorWindow + 1);
            }

            case Win32::Messages::Notify:
            {
                auto some_item = reinterpret_cast<Win32::LPNMHDR>(lParam);

                if (some_item->idFrom == IDC_EXIT_BUTTON && some_item->code == Win32::Messages::CustomDraw)
                {
                    auto item = reinterpret_cast<Win32::LPNMCUSTOMDRAW>(some_item);

                    if (item->uItemState & Win32::CdisSelected)
                    {
                        //Select our color when the button is selected
                        if (selectbrush == nullptr)
                            selectbrush = CreateGradientBrush(Win32::GetRGB(180, 0, 0), Win32::GetRGB(255, 180, 0), item);

                        //Create pen for button border
                        auto pen = Win32::CreatePen(Win32::PsInsideFrame, 0, Win32::GetRGB(0, 0, 0));

                        //Select our brush into hDC
                        auto old_pen = Win32::SelectObject(item->hdc, pen);
                        auto old_brush = Win32::SelectObject(item->hdc, selectbrush);

                        //If you want rounded button, then use this, otherwise use FillRect().
                        Win32::RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 5, 5);

                        //Clean up
                        Win32::SelectObject(item->hdc, old_pen);
                        Win32::SelectObject(item->hdc, old_brush);
                        Win32::DeleteObject(pen);

                        //Now, I don't want to do anything else myself (draw text) so I use this value for return:
                        return Win32::DoDefault;
                        //Let's say I wanted to draw text and stuff, then I would have to do it before return with
                        //DrawText() or other function and return CDRF_SKIPDEFAULT
                    }
                    else
                    {
                        if (item->uItemState & Win32::CdisHot) //Our mouse is over the button
                        {
                            //Select our color when the mouse hovers our button
                            if (hotbrush == nullptr)
                                hotbrush = CreateGradientBrush(Win32::GetRGB(255, 230, 0), Win32::GetRGB(245, 0, 0), item);

                            auto pen = Win32::CreatePen(Win32::PsInsideFrame, 0, Win32::GetRGB(0, 0, 0));

                            auto old_pen = Win32::SelectObject(item->hdc, pen);
                            auto old_brush = Win32::SelectObject(item->hdc, hotbrush);

                            Win32::RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 5, 5);

                            Win32::SelectObject(item->hdc, old_pen);
                            Win32::SelectObject(item->hdc, old_brush);
                            Win32::DeleteObject(pen);

                            return Win32::DoDefault;
                        }

                        //Select our color when our button is doing nothing
                        if (defaultbrush == nullptr)
                            defaultbrush = CreateGradientBrush(Win32::GetRGB(255, 180, 0), Win32::GetRGB(180, 0, 0), item);

                        auto pen = Win32::CreatePen(Win32::PsInsideFrame, 0, Win32::GetRGB(0, 0, 0));

                        auto old_pen = Win32::SelectObject(item->hdc, pen);
                        auto old_brush = Win32::SelectObject(item->hdc, defaultbrush);

                        Win32::RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 5, 5);

                        Win32::SelectObject(item->hdc, old_pen);
                        Win32::SelectObject(item->hdc, old_brush);
                        Win32::DeleteObject(pen);

                        return Win32::DoDefault;
                    }
                }
                else if (some_item->idFrom == IDC_PUSHLIKE_BUTTON && some_item->code == Win32::Messages::CustomDraw)
                {
                    auto item = reinterpret_cast<Win32::LPNMCUSTOMDRAW>(some_item);

                    if (Win32::IsDlgButtonChecked(hwnd, some_item->idFrom))
                    {
                        if (item->uItemState & Win32::CdisHot)
                        {

                            if (push_hotbrush1 == nullptr)
                                push_hotbrush1 = CreateGradientBrush(Win32::GetRGB(0, 0, 245), Win32::GetRGB(0, 230, 255), item);

                            auto pen = Win32::CreatePen(Win32::PsInsideFrame, 0, Win32::GetRGB(0, 0, 0));

                            auto old_pen = Win32::SelectObject(item->hdc, pen);
                            auto old_brush = Win32::SelectObject(item->hdc, push_hotbrush1);

                            Win32::RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 10, 10);

                            Win32::SelectObject(item->hdc, old_pen);
                            Win32::SelectObject(item->hdc, old_brush);
                            Win32::DeleteObject(pen);

                            return Win32::DoDefault;
                        }

                        if (push_checkedbrush == nullptr)
                            push_checkedbrush = CreateGradientBrush(Win32::GetRGB(0, 0, 180), Win32::GetRGB(0, 222, 200), item);

                        auto pen = Win32::CreatePen(Win32::PsInsideFrame, 0, Win32::GetRGB(0, 0, 0));

                        auto old_pen = Win32::SelectObject(item->hdc, pen);
                        auto old_brush = Win32::SelectObject(item->hdc, push_checkedbrush);

                        Win32::RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 10, 10);

                        Win32::SelectObject(item->hdc, old_pen);
                        Win32::SelectObject(item->hdc, old_brush);
                        Win32::DeleteObject(pen);

                        return Win32::DoDefault;
                    }
                    else
                    {
                        if (item->uItemState & Win32::CdisHot)
                        {
                            if (push_hotbrush2 == nullptr)
                                push_hotbrush2 = CreateGradientBrush(Win32::GetRGB(255, 230, 0), Win32::GetRGB(245, 0, 0), item);

                            auto pen = Win32::CreatePen(Win32::PsInsideFrame, 0, Win32::GetRGB(0, 0, 0));

                            auto old_pen = Win32::SelectObject(item->hdc, pen);
                            auto old_brush = Win32::SelectObject(item->hdc, push_hotbrush2);

                            Win32::RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 10, 10);

                            Win32::SelectObject(item->hdc, old_pen);
                            Win32::SelectObject(item->hdc, old_brush);
                            Win32::DeleteObject(pen);

                            return Win32::DoDefault;
                        }

                        if (push_uncheckedbrush == nullptr)
                            push_uncheckedbrush = CreateGradientBrush(Win32::GetRGB(255, 180, 0), Win32::GetRGB(180, 0, 0), item);

                        auto pen = Win32::CreatePen(Win32::PsInsideFrame, 0, Win32::GetRGB(0, 0, 0));

                        auto old_pen = Win32::SelectObject(item->hdc, pen);
                        auto old_brush = Win32::SelectObject(item->hdc, defaultbrush);

                        Win32::RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 10, 10);

                        Win32::SelectObject(item->hdc, old_pen);
                        Win32::SelectObject(item->hdc, old_brush);
                        Win32::DeleteObject(pen);

                        return Win32::DoDefault;
                    }
                }

                return Win32::DoDefault;
            }

            case Win32::Messages::Destroy:
            {
                if (defaultbrush)
                    Win32::DeleteObject(defaultbrush);
                if (selectbrush)
                    Win32::DeleteObject(selectbrush);
                if (hotbrush)
                    Win32::DeleteObject(hotbrush);
                if (push_checkedbrush)
                    Win32::DeleteObject(push_checkedbrush);
                if (push_hotbrush1)
                    Win32::DeleteObject(push_hotbrush1);
                if (push_hotbrush2)
                    Win32::DeleteObject(push_hotbrush2);
                if (push_uncheckedbrush)
                    Win32::DeleteObject(push_uncheckedbrush);
                Win32::PostQuitMessage(0);
                return 0;
            }

            case Win32::Messages::Close:
            {
                Win32::DestroyWindow(hwnd);
                break;
            }

            default:
                return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
        }
        return 0;
    }

    export auto Run() -> int
    {
        const wchar_t ClassName[] = L"Main_Window";
        auto wc = Win32::WNDCLASSEXW{
            .cbSize = sizeof(Win32::WNDCLASSEX),
            .lpfnWndProc =
                MainWindow,
                /*[](Win32::HWND hwnd, Win32::UINT msg, Win32::WPARAM wParam, Win32::LPARAM lParam) -> long long
                {
                    switch (msg)
                    {
                        case Win32::Messages::Close:
                            Win32::PostQuitMessage(0);
                            return 0;

                        default:
                            return Win32::DefWindowProcW(hwnd, msg, wParam, lParam);
                    }
                    return 0;
                },*/
            .hInstance = Win32::GetModuleHandleW(nullptr),
            .hIcon = Win32::LoadIconW(nullptr, Win32::IdiApplication()),
            .hCursor = Win32::LoadCursorW(nullptr, Win32::Cursors::Arrow),
            .hbrBackground = Win32::GetSysColorBrush(Win32::ColorWindow + 1),
            .lpszClassName = ClassName,
            .hIconSm = Win32::LoadIconW(nullptr, Win32::IdiApplication())
        };

        if (!Win32::RegisterClassExW(&wc))
        {
            Win32::MessageBoxW(nullptr, L"Window Registration Failed!", L"Error", Win32::MessageBoxes::IconExclamation | Win32::MessageBoxes::OK);
            std::terminate();
        }

        auto hwnd = Win32::CreateWindowExW(
            Win32::ExtendedStyles::ClientEdge,
            ClassName,
            L"Window",
            Win32::Styles::OverlappedWindow,
            Win32::Cw_UseDefault,
            Win32::Cw_UseDefault,
            368,
            248,
            nullptr,
            nullptr,
            Win32::GetModuleHandleW(nullptr),
            nullptr
        );

        if (hwnd == nullptr)
        {
            Win32::MessageBoxW(nullptr, L"Window Creation Failed!", L"Error!", Win32::MessageBoxes::IconExclamation | Win32::MessageBoxes::OK);
            std::terminate();
        }

        Win32::ShowWindow(hwnd, Win32::Sw_ShowDefault);
        Win32::UpdateWindow(hwnd);


        auto msg = Win32::MSG{};
        while (Win32::GetMessageW(&msg, nullptr, 0, 0) > 0)
        {
            Win32::TranslateMessage(&msg);
            Win32::DispatchMessageW(&msg);
        }

        return msg.message;
    }
}
