// Adapted from https://learn.microsoft.com/en-us/windows/win32/secbp/asking-the-user-for-credentials
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wincred.h>

import std;

#pragma comment(lib, "Credui.lib")

void Ask()
{
    std::array<wchar_t, CREDUI_MAX_USERNAME_LENGTH + 1> pszName;
    std::array<wchar_t, CREDUI_MAX_PASSWORD_LENGTH + 1> pszPwd;
    BOOL fSave = false;

    //  Ensure that MessageText and CaptionText identify what credentials
    //  to use and which application requires them.
    CREDUI_INFO cui
    {
        .cbSize = sizeof(CREDUI_INFO),
        .hwndParent = nullptr,
        .pszMessageText = L"Enter administrator account information",
        .pszCaptionText = L"CredUITest",
        .hbmBanner = nullptr
    };

    // https://learn.microsoft.com/en-us/windows/win32/api/wincred/nf-wincred-creduipromptforcredentialsw
    DWORD dwErr = CredUIPromptForCredentialsW(
        &cui,                                   // CREDUI_INFO structure
        L"TheServer",                           // Target for credentials, usually a server
        nullptr,                                // Reserved
        0,                                      // Reason
        pszName.data(),                         // User name
        static_cast<ULONG>(pszName.size()),     // Max number of char for user name
        pszPwd.data(),                          // Password
        static_cast<ULONG>(pszPwd.size()),      // Max number of char for password
        &fSave,                                 // State of save check box
        CREDUI_FLAGS_GENERIC_CREDENTIALS | CREDUI_FLAGS_ALWAYS_SHOW_UI | CREDUI_FLAGS_DO_NOT_PERSIST
    );
    if (dwErr)
    {
        std::println("Failed.");
        return;
    }

    //  Put code that uses the credentials here.
    //  When you have finished using the credentials,
    //  erase them from memory.
    std::println("Success.");
    std::fill(pszName.begin(), pszName.end(), 0);
    std::fill(pszPwd.begin(), pszPwd.end(), 0);
}

int main()
{
    Ask();
    return 0;
}
