export module SspiExample:Client;
import std;
import Win32;
import :Common;

#pragma comment(lib, "Ws2_32.lib")

export namespace SspiExample::Client
{
    Win32::CredHandle hCred;
    Win32::_SecHandle  hcText;

    constexpr auto BIG_BUFF = 2048;
    constexpr std::wstring_view ServerName = L"www.google.com";
    constexpr auto g_usPort = 2000;

    Win32::BOOL ConnectAuthSocket(
        Win32::SOCKET* s,
        Win32::CredHandle* hCred,
        Win32::_SecHandle* hcText
    )
    {
        return true;
    }

    void Run()
	{
        Win32::SOCKET            Client_Socket;
        Win32::BYTE              Data[BIG_BUFF];
        Win32::PCHAR             pMessage;
        Win32::WSADATA           wsaData;
        Win32::CredHandle        hCred;
        struct _SecHandle hCtxt;
        Win32::SECURITY_STATUS   ss;
        Win32::DWORD             cbRead;
        Win32::ULONG             cbMaxSignature;
        Win32::ULONG             cbSecurityTrailer;
        Win32::SecPkgContext_Sizes            SecPkgContextSizes;
        Win32::SecPkgContext_NegotiationInfoW  SecPkgNegInfo;

        if (Win32::WSAStartup(Win32::MakeWord(), &wsaData))
        {
            MyHandleError("Could not initialize winsock ");
        }

        if (!ConnectAuthSocket(
            &Client_Socket,
            &hCred,
            &hcText))
        {
            MyHandleError("Authenticated server connection ");
        }
	}
}