#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Adapted from Microsoft DesktopToasts sample in windows-classic-samples: https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/DesktopToasts

#include <SDKDDKVer.h>
#include <Windows.h>
#include <Psapi.h>
#include <strsafe.h>
#include <ShObjIdl.h>
#include <Shlobj.h>
#include <Pathcch.h>
#include <propvarutil.h>
#include <propkey.h>
#include <wrl.h>
#include <wrl\wrappers\corewrappers.h>
#include <windows.ui.notifications.h>
#include "NotificationActivationCallback.h"
import std;

//  Name:     System.AppUserModel.ToastActivatorCLSID -- PKEY_AppUserModel_ToastActivatorCLSID
//  Type:     Guid -- VT_CLSID
//  FormatID: {9F4C2855-9F79-4B39-A8D0-E1D42DE1D5F3}, 26
//  
//  Used to CoCreate an INotificationActivationCallback interface to notify about toast activations.
EXTERN_C const PROPERTYKEY DECLSPEC_SELECTANY PKEY_AppUserModel_ToastActivatorCLSID = { { 0x9F4C2855, 0x9F79, 0x4B39, { 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3 } }, 26 };

struct CoTaskMemStringTraits
{
    typedef PWSTR Type;

    inline static bool Close(_In_ Type h) throw() { ::CoTaskMemFree(h); return true; }

    inline static Type GetInvalidValue() throw() { return nullptr; }
};
typedef Microsoft::WRL::Wrappers::HandleT<CoTaskMemStringTraits> CoTaskMemString;

const wchar_t AppId[] = L"Microsoft.Samples.DesktopToasts";

class DesktopToastsApp
{
public:
    static DesktopToastsApp* GetInstance()
    {
        return s_currentInstance;
    }

    DesktopToastsApp();
    ~DesktopToastsApp();
    HRESULT Initialize(_In_ HINSTANCE hInstance);
    void RunMessageLoop();

    HRESULT SetMessage(PCWSTR message);

private:
    HRESULT RegisterAppForNotificationSupport();
    HRESULT InstallShortcut(_In_ PCWSTR shortcutPath, _In_ PCWSTR exePath);
    HRESULT RegisterComServer(_In_ PCWSTR exePath);

    HRESULT RegisterActivator();
    void UnregisterActivator();

    static LRESULT CALLBACK WndProc(
        _In_ HWND hWnd,
        _In_ UINT message,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam
        );

    HRESULT DisplayToast();

    HRESULT CreateToastXml(
        _In_ ABI::Windows::UI::Notifications::IToastNotificationManagerStatics* toastManager,
        _COM_Outptr_ ABI::Windows::Data::Xml::Dom::IXmlDocument** xml
        );

    HRESULT CreateToast(
        _In_ ABI::Windows::UI::Notifications::IToastNotificationManagerStatics* toastManager,
        _In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml
        );
    HRESULT SetImageSrc(
        _In_ PCWSTR imagePath,
        _In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* toastXml
        );
    HRESULT SetTextValues(
        _In_reads_(textValuesCount) const PCWSTR* textValues,
        _In_ UINT32 textValuesCount,
        _Inout_ ABI::Windows::Data::Xml::Dom::IXmlDocument* toastXml
        );
    HRESULT SetNodeValueString(
        _In_ HSTRING onputString,
        _Inout_ ABI::Windows::Data::Xml::Dom::IXmlNode* node,
        _In_ ABI::Windows::Data::Xml::Dom::IXmlDocument* xml
        );

    HWND m_hwnd = nullptr;
    HWND m_hEdit = nullptr;

    static const WORD HM_TEXTBUTTON = 1;

    static DesktopToastsApp* s_currentInstance;
};

DesktopToastsApp* DesktopToastsApp::s_currentInstance = nullptr;

// For the app to be activated from Action Center, it needs to provide a COM server to be called
// when the notification is activated.  The CLSID of the object needs to be registered with the
// OS via its shortcut so that it knows who to call later.
class DECLSPEC_UUID("23A5B06E-20BB-4E7E-A0AC-6982ED6A6041") NotificationActivator WrlSealed
    : public Microsoft::WRL::RuntimeClass < Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    INotificationActivationCallback > WrlFinal
{
public:
    virtual HRESULT STDMETHODCALLTYPE Activate(
        _In_ LPCWSTR /*appUserModelId*/,
        _In_ LPCWSTR /*invokedArgs*/,
        /*_In_reads_(dataCount)*/ const NOTIFICATION_USER_INPUT_DATA* /*data*/,
        ULONG /*dataCount*/) override
    {
        return DesktopToastsApp::GetInstance()->SetMessage(L"NotificationActivator - The user clicked on the toast.");
    }
};
CoCreatableClass(NotificationActivator);


// Main function
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
    Microsoft::WRL::Wrappers::RoInitializeWrapper winRtInitializer(RO_INIT_MULTITHREADED);

    HRESULT hr = winRtInitializer;
    if (SUCCEEDED(hr))
    {
        DesktopToastsApp app;
        hr = app.Initialize(hInstance);
        if (SUCCEEDED(hr))
        {
            app.RunMessageLoop();
        }
    }

    return SUCCEEDED(hr);
}

DesktopToastsApp::DesktopToastsApp()
{
    s_currentInstance = this;
}

DesktopToastsApp::~DesktopToastsApp()
{
    UnregisterActivator();
    s_currentInstance = nullptr;
}

// In order to display toasts, a desktop application must have a shortcut on the Start menu.
// Also, an AppUserModelID must be set on that shortcut.
//
// For the app to be activated from Action Center, it needs to register a COM server with the OS
// and register the CLSID of that COM server on the shortcut.
//
// The shortcut should be created as part of the installer. The following code shows how to create
// a shortcut and assign the AppUserModelID and ToastActivatorCLSID properties using Windows APIs.
//
// Included in this project is a wxs file that be used with the WiX toolkit
// to make an installer that creates the necessary shortcut. One or the other should be used.
//
// This sample doesn't clean up the shortcut or COM registration.

HRESULT DesktopToastsApp::RegisterAppForNotificationSupport()
{
    CoTaskMemString appData;
    auto hr = ::SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, appData.GetAddressOf());
    if (FAILED(hr))
        return hr;

    wchar_t shortcutPath[MAX_PATH];
    hr = ::PathCchCombine(shortcutPath, ARRAYSIZE(shortcutPath), appData.Get(), LR"(Microsoft\Windows\Start Menu\Programs\Desktop Toasts App.lnk)");
    if (FAILED(hr))
        return hr;

    DWORD attributes = ::GetFileAttributes(shortcutPath);
    bool fileExists = attributes < 0xFFFFFFF;
    if (fileExists)
        return hr;

    wchar_t exePath[MAX_PATH];
    DWORD charWritten = ::GetModuleFileName(nullptr, exePath, ARRAYSIZE(exePath));
    hr = charWritten > 0 ? S_OK : HRESULT_FROM_WIN32(::GetLastError());
    if (FAILED(hr))
        return hr;

    hr = InstallShortcut(shortcutPath, exePath);
    if (FAILED(hr))
        return hr;

    hr = RegisterComServer(exePath);

    return hr;
}

_Use_decl_annotations_
HRESULT DesktopToastsApp::InstallShortcut(PCWSTR shortcutPath, PCWSTR exePath)
{
    Microsoft::WRL::ComPtr<IShellLink> shellLink;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink));
    if (FAILED(hr))
        return hr;

    hr = shellLink->SetPath(exePath);
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<IPropertyStore> propertyStore;

    hr = shellLink.As(&propertyStore);
    if (FAILED(hr))
        return hr;

    PROPVARIANT propVar;
    propVar.vt = VT_LPWSTR;
    propVar.pwszVal = const_cast<PWSTR>(AppId); // for _In_ scenarios, we don't need a copy
    hr = propertyStore->SetValue(PKEY_AppUserModel_ID, propVar);
    if (FAILED(hr))
        return hr;

    propVar.vt = VT_CLSID;
    propVar.puuid = const_cast<CLSID*>(&__uuidof(NotificationActivator));
    hr = propertyStore->SetValue(PKEY_AppUserModel_ToastActivatorCLSID, propVar);
    if (FAILED(hr))
        return hr;

    hr = propertyStore->Commit();
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<IPersistFile> persistFile;
    hr = shellLink.As(&persistFile);
    if (FAILED(hr))
        return hr;
   
    hr = persistFile->Save(shortcutPath, TRUE);
 
    return hr;
}

_Use_decl_annotations_
HRESULT DesktopToastsApp::RegisterComServer(PCWSTR exePath)
{
    // We don't need to worry about overflow here as ::GetModuleFileName won't
    // return anything bigger than the max file system path (much fewer than max of DWORD).
    DWORD dataSize = static_cast<DWORD>((::wcslen(exePath) + 1)  * sizeof(WCHAR));

    // In this sample, the app UI is registered to launch when the COM callback is needed.
    // Other options might be to launch a background process instead that then decides to launch
    // the UI if needed by that particular notification.
    return HRESULT_FROM_WIN32(::RegSetKeyValue(
        HKEY_CURRENT_USER,
        LR"(SOFTWARE\Classes\CLSID\{23A5B06E-20BB-4E7E-A0AC-6982ED6A6041}\LocalServer32)",
        nullptr,
        REG_SZ,
        reinterpret_cast<const BYTE*>(exePath),
        dataSize));
}

// Register activator for notifications
HRESULT DesktopToastsApp::RegisterActivator()
{
    // Module<OutOfProc> needs a callback registered before it can be used.
    // Since we don't care about when it shuts down, we'll pass an empty lambda here.
    Microsoft::WRL::Module<Microsoft::WRL::OutOfProc>::Create([] {});

    // If a local server process only hosts the COM object then COM expects
    // the COM server host to shutdown when the references drop to zero.
    // Since the user might still be using the program after activating the notification,
    // we don't want to shutdown immediately.  Incrementing the object count tells COM that
    // we aren't done yet.
    Microsoft::WRL::Module<Microsoft::WRL::OutOfProc>::GetModule().IncrementObjectCount();

    return Microsoft::WRL::Module<Microsoft::WRL::OutOfProc>::GetModule().RegisterObjects();
}

// Unregister our activator COM object
void DesktopToastsApp::UnregisterActivator()
{
    Microsoft::WRL::Module<Microsoft::WRL::OutOfProc>::GetModule().UnregisterObjects();

    Microsoft::WRL::Module<Microsoft::WRL::OutOfProc>::GetModule().DecrementObjectCount();
}

// Prepare the main window
_Use_decl_annotations_
HRESULT DesktopToastsApp::Initialize(HINSTANCE hInstance)
{
    HRESULT hr = RegisterAppForNotificationSupport();
    if (FAILED(hr))
        return hr;

    WNDCLASSEX wcex = { sizeof(wcex) };
    // Register window class
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = DesktopToastsApp::WndProc;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = L"DesktopToastsApp";
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    ::RegisterClassEx(&wcex);

    // Create window
    m_hwnd = CreateWindow(
        L"DesktopToastsApp",
        L"Desktop Toasts Demo App",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        350, 200,
        nullptr, nullptr,
        hInstance, this);

    hr = m_hwnd ? S_OK : E_FAIL;
    if (FAILED(hr))
        return hr;

    ::CreateWindow(
        L"BUTTON",
        L"View Text Toast",
        BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
        0, 0, 150, 25,
        m_hwnd, reinterpret_cast<HMENU>(HM_TEXTBUTTON),
        hInstance, nullptr);
    m_hEdit = ::CreateWindow(
        L"EDIT",
        L"Whatever action you take on the displayed toast will be shown here.",
        ES_LEFT | ES_MULTILINE | ES_READONLY | WS_CHILD | WS_VISIBLE | WS_BORDER,
        0, 30, 300, 50,
        m_hwnd, nullptr,
        hInstance, nullptr);

    ::ShowWindow(m_hwnd, SW_SHOWNORMAL);
    ::UpdateWindow(m_hwnd);

    hr = RegisterActivator();

    return hr;
}

// Standard message loop
void DesktopToastsApp::RunMessageLoop()
{
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

HRESULT DesktopToastsApp::SetMessage(PCWSTR message)
{
    ::SetForegroundWindow(m_hwnd);

    ::SendMessage(m_hEdit, WM_SETTEXT, reinterpret_cast<WPARAM>(nullptr), reinterpret_cast<LPARAM>(message));

    return S_OK;
}

// Display the toast using classic COM. Note that is also possible to create and
// display the toast using the new C++ /ZW options (using handles, COM wrappers, etc.)
HRESULT DesktopToastsApp::DisplayToast()
{
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotificationManagerStatics> toastStatics;
    HRESULT hr = Windows::Foundation::GetActivationFactory(
        Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(),
        &toastStatics);
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlDocument> toastXml;
    hr = CreateToastXml(toastStatics.Get(), &toastXml);
    if (FAILED(hr))
        return hr;

    hr = CreateToast(toastStatics.Get(), toastXml.Get());

    return hr;
}

// Create the toast XML from a template
_Use_decl_annotations_
HRESULT DesktopToastsApp::CreateToastXml(ABI::Windows::UI::Notifications::IToastNotificationManagerStatics* toastManager, ABI::Windows::Data::Xml::Dom::IXmlDocument** inputXml)
{
    *inputXml = nullptr;

    // Retrieve the template XML
    HRESULT hr = toastManager->GetTemplateContent(ABI::Windows::UI::Notifications::ToastTemplateType_ToastImageAndText04, inputXml);
    if (FAILED(hr))
        return hr;

    std::filesystem::path imagePath = std::filesystem::absolute({ L".\\toastImageAndText.png" });

    hr = SetImageSrc(imagePath.c_str(), *inputXml);
    if (FAILED(hr))
        return hr;

    const PCWSTR textValues[] = {
        L"Line 1",
        L"Line 2",
        L"Line 3"
    };
    hr = SetTextValues(textValues, ARRAYSIZE(textValues), *inputXml);

    return hr;
}

// Set the value of the "src" attribute of the "image" node
_Use_decl_annotations_
HRESULT DesktopToastsApp::SetImageSrc(PCWSTR imagePath, ABI::Windows::Data::Xml::Dom::IXmlDocument* toastXml)
{
    wchar_t imageSrcUri[MAX_PATH];
    DWORD size = ARRAYSIZE(imageSrcUri);

    HRESULT hr = ::UrlCreateFromPath(imagePath, imageSrcUri, &size, 0);
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNodeList> nodeList;
    hr = toastXml->GetElementsByTagName(Microsoft::WRL::Wrappers::HStringReference(L"image").Get(), &nodeList);
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> imageNode;
    hr = nodeList->Item(0, &imageNode);
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNamedNodeMap> attributes;
    hr = imageNode->get_Attributes(&attributes);
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> srcAttribute;
    hr = attributes->GetNamedItem(Microsoft::WRL::Wrappers::HStringReference(L"src").Get(), &srcAttribute);
    if (FAILED(hr))
        return hr;

    hr = SetNodeValueString(Microsoft::WRL::Wrappers::HStringReference(imageSrcUri).Get(), srcAttribute.Get(), toastXml);

    return hr;
}

// Set the values of each of the text nodes
_Use_decl_annotations_
HRESULT DesktopToastsApp::SetTextValues(const PCWSTR* textValues, UINT32 textValuesCount, ABI::Windows::Data::Xml::Dom::IXmlDocument* toastXml)
{
    Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNodeList> nodeList;
    HRESULT hr = toastXml->GetElementsByTagName(Microsoft::WRL::Wrappers::HStringReference(L"text").Get(), &nodeList);
    if (FAILED(hr))
        return hr;

    UINT32 nodeListLength;
    hr = nodeList->get_Length(&nodeListLength);
    if (FAILED(hr))
        return hr;

    // If a template was chosen with fewer text elements, also change the amount of strings
    // passed to this method.
    hr = textValuesCount <= nodeListLength ? S_OK : E_INVALIDARG;
    if (FAILED(hr))
        return hr;

    for (UINT32 i = 0; i < textValuesCount; i++)
    {
        Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> textNode;
        hr = nodeList->Item(i, &textNode);
        if (SUCCEEDED(hr))
            hr = SetNodeValueString(Microsoft::WRL::Wrappers::HStringReference(textValues[i]).Get(), textNode.Get(), toastXml);
    }

    return hr;
}

_Use_decl_annotations_
HRESULT DesktopToastsApp::SetNodeValueString(HSTRING inputString, ABI::Windows::Data::Xml::Dom::IXmlNode* node, ABI::Windows::Data::Xml::Dom::IXmlDocument* xml)
{
    Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlText> inputText;
    HRESULT hr = xml->CreateTextNode(inputString, &inputText);
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> inputTextNode;
    hr = inputText.As(&inputTextNode);
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlNode> appendedChild;
    hr = node->AppendChild(inputTextNode.Get(), &appendedChild);

    return hr;
}

// Create and display the toast
_Use_decl_annotations_
HRESULT DesktopToastsApp::CreateToast(ABI::Windows::UI::Notifications::IToastNotificationManagerStatics* toastManager, ABI::Windows::Data::Xml::Dom::IXmlDocument* xml)
{
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotifier> notifier;
    HRESULT hr = toastManager->CreateToastNotifierWithId(Microsoft::WRL::Wrappers::HStringReference(AppId).Get(), &notifier);
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotificationFactory> factory;
    hr = Windows::Foundation::GetActivationFactory(
        Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotification).Get(),
        &factory);
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotification> toast;
    hr = factory->CreateToastNotification(xml, &toast);
    if (FAILED(hr))
        return hr;

    // Register the event handlers
    //
    // These handlers are called asynchronously.  This sample doesn't handle the
    // the fact that these events could be raised after the app object has already
    // been decontructed.
    EventRegistrationToken activatedToken, dismissedToken, failedToken;

    using namespace ABI::Windows::Foundation;

    hr = toast->add_Activated(
        Microsoft::WRL::Callback<
            Microsoft::WRL::Implements<
                Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification*, IInspectable*>>>
        (
            [](ABI::Windows::UI::Notifications::IToastNotification*, IInspectable*)
            {
                // When the user clicks or taps on the toast, the registered
                // COM object is activated, and the Activated event is raised.
                // There is no guarantee which will happen first. If the COM
                // object is activated first, then this message may not show.
                DesktopToastsApp::GetInstance()->SetMessage(L"The user clicked on the toast.");
                return S_OK;
            }).Get(),
            &activatedToken
        );

    if (FAILED(hr))
        return hr;

    hr = toast->add_Dismissed(
        Microsoft::WRL::Callback<
            Microsoft::WRL::Implements<
                Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification*, ABI::Windows::UI::Notifications::ToastDismissedEventArgs*>
            >
        >(
        [](ABI::Windows::UI::Notifications::IToastNotification*, ABI::Windows::UI::Notifications::IToastDismissedEventArgs* e)
        {
                ABI::Windows::UI::Notifications::ToastDismissalReason reason;
            if (FAILED(e->get_Reason(&reason)))
                return S_OK;

            PCWSTR outputText;
            switch (reason)
            {
                case ABI::Windows::UI::Notifications::ToastDismissalReason_ApplicationHidden:
                    outputText = L"The application hid the toast using ToastNotifier.hide()";
                    break;
                case ABI::Windows::UI::Notifications::ToastDismissalReason_UserCanceled:
                    outputText = L"The user dismissed this toast";
                    break;
                case ABI::Windows::UI::Notifications::ToastDismissalReason_TimedOut:
                    outputText = L"The toast has timed out";
                    break;
                default:
                    outputText = L"Toast not activated";
                    break;
            }

            DesktopToastsApp::GetInstance()->SetMessage(outputText);

            return S_OK;
        }).Get(),
        &dismissedToken
    );

    if (FAILED(hr))
        return hr;

    hr = toast->add_Failed(
        Microsoft::WRL::Callback<
            Microsoft::WRL::Implements<
                Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
                ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification*, ABI::Windows::UI::Notifications::ToastFailedEventArgs*>>>(
        [](ABI::Windows::UI::Notifications::IToastNotification*, ABI::Windows::UI::Notifications::IToastFailedEventArgs* /*e */)
        {
            DesktopToastsApp::GetInstance()->SetMessage(L"The toast encountered an error.");
            return S_OK;
        }).Get(),
        &failedToken
    );

    if (SUCCEEDED(hr))
        hr = notifier->Show(toast.Get());

    return hr;
}

// Standard window procedure
_Use_decl_annotations_
LRESULT CALLBACK DesktopToastsApp::WndProc(HWND hwnd, UINT32 message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CREATE)
    {
        auto pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        auto app = reinterpret_cast<DesktopToastsApp *>(pcs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
        return 1;
    }

    auto app = reinterpret_cast<DesktopToastsApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!app)
        return DefWindowProc(hwnd, message, wParam, lParam);

    switch (message)
    {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
                case DesktopToastsApp::HM_TEXTBUTTON:
                    app->DisplayToast();
                    break;
                default:
                    return DefWindowProc(hwnd, message, wParam, lParam);
            }
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 1;
        }

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
}
