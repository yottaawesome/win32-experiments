/*
 * Sample to show using WinSock with SSPI and TLS to establish 
 * secure comms with a remote server. This code was reproduced 
 * originally from https://web.archive.org/web/20210116110926/http://www.coastrd.com/c-schannel-smtp
 * with various changes to make it compile on VS 2022.
 * See also https://learn.microsoft.com/en-us/windows/win32/secauthn/using-sspi-with-a-windows-sockets-client
 * and https://stackoverflow.com/questions/2032056/compilable-c-code-to-implement-a-secure-sll-tls-client-using-ms-sspi.
 * -Vasilios Magriplis, 27/11/2022
*/

#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(disable : 4996)

// Compiles with Visual Studio 2008 for Windows

// This C example is designed as more of a guide than a library to be plugged into an application
// That module required a couple of major re-writes and is available upon request
// The Basic example has tips to the direction you should take
// This will work with connections on port 587 that upgrade a plain text session to an encrypted session with STARTTLS as covered here.

// TLSclient.c - SSPI Schannel gmail TLS connection example

#define SECURITY_WIN32
#define IO_BUFFER_SIZE  0x10000
#define DLL_NAME TEXT("Secur32.dll")
#define NT4_DLL_NAME TEXT("Security.dll")

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <schannel.h>
#include <security.h>
#include <sspi.h>

#pragma comment(lib, "WSock32.Lib")
#pragma comment(lib, "Crypt32.Lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Secur32.lib")

// Globals.
BOOL    fVerbose = FALSE; // FALSE; // TRUE;


INT     iPortNumber = 465; // gmail TLS
LPSTR   pszServerName = LPSTR("smtp.gmail.com"); // DNS name of server
LPSTR   pszUser = 0; // if specified, a certificate in "MY" store is searched for

DWORD   dwProtocol = SP_PROT_TLS1; // SP_PROT_TLS1; // SP_PROT_PCT1; SP_PROT_SSL2; SP_PROT_SSL3; 0=default
ALG_ID  aiKeyExch = 0; // = default; CALG_DH_EPHEM; CALG_RSA_KEYX;

BOOL    fUseProxy = FALSE;
LPSTR   pszProxyServer = LPSTR("proxy");
INT     iProxyPort = 80;

HCERTSTORE hMyCertStore = NULL;
HMODULE g_hSecurity = NULL;

SCHANNEL_CRED SchannelCred;
PSecurityFunctionTable g_pSSPI;



/*****************************************************************************/
static void DisplayWinVerifyTrustError(DWORD Status)
{
    LPSTR pszName = NULL;

    switch (Status)
    {
    case CERT_E_EXPIRED:                pszName = LPSTR("CERT_E_EXPIRED");                 break;
    case CERT_E_VALIDITYPERIODNESTING:  pszName = LPSTR("CERT_E_VALIDITYPERIODNESTING");   break;
    case CERT_E_ROLE:                   pszName = LPSTR("CERT_E_ROLE");                    break;
    case CERT_E_PATHLENCONST:           pszName = LPSTR("CERT_E_PATHLENCONST");            break;
    case CERT_E_CRITICAL:               pszName = LPSTR("CERT_E_CRITICAL");                break;
    case CERT_E_PURPOSE:                pszName = LPSTR("CERT_E_PURPOSE");                 break;
    case CERT_E_ISSUERCHAINING:         pszName = LPSTR("CERT_E_ISSUERCHAINING");          break;
    case CERT_E_MALFORMED:              pszName = LPSTR("CERT_E_MALFORMED");               break;
    case CERT_E_UNTRUSTEDROOT:          pszName = LPSTR("CERT_E_UNTRUSTEDROOT");           break;
    case CERT_E_CHAINING:               pszName = LPSTR("CERT_E_CHAINING");                break;
    case TRUST_E_FAIL:                  pszName = LPSTR("TRUST_E_FAIL");                   break;
    case CERT_E_REVOKED:                pszName = LPSTR("CERT_E_REVOKED");                 break;
    case CERT_E_UNTRUSTEDTESTROOT:      pszName = LPSTR("CERT_E_UNTRUSTEDTESTROOT");       break;
    case CERT_E_REVOCATION_FAILURE:     pszName = LPSTR("CERT_E_REVOCATION_FAILURE");      break;
    case CERT_E_CN_NO_MATCH:            pszName = LPSTR("CERT_E_CN_NO_MATCH");             break;
    case CERT_E_WRONG_USAGE:            pszName = LPSTR("CERT_E_WRONG_USAGE");             break;
    default:                            pszName = LPSTR("(unknown)");                      break;
    }
    printf("Error 0x%x (%s) returned by CertVerifyCertificateChainPolicy!\n", Status, pszName);
}


/*****************************************************************************/
static void DisplayWinSockError(DWORD ErrCode)
{
    LPSTR pszName = NULL; // http://www.sockets.com/err_lst1.htm#WSANO_DATA

    switch (ErrCode) // http://msdn.microsoft.com/en-us/library/ms740668(VS.85).aspx
    {
    case     10035:  pszName = LPSTR("WSAEWOULDBLOCK    "); break;
    case     10036:  pszName = LPSTR("WSAEINPROGRESS    "); break;
    case     10037:  pszName = LPSTR("WSAEALREADY       "); break;
    case     10038:  pszName = LPSTR("WSAENOTSOCK       "); break;
    case     10039:  pszName = LPSTR("WSAEDESTADDRREQ   "); break;
    case     10040:  pszName = LPSTR("WSAEMSGSIZE       "); break;
    case     10041:  pszName = LPSTR("WSAEPROTOTYPE     "); break;
    case     10042:  pszName = LPSTR("WSAENOPROTOOPT    "); break;
    case  10043:  pszName = LPSTR("WSAEPROTONOSUPPORT"); break;
    case  10044:  pszName = LPSTR("WSAESOCKTNOSUPPORT"); break;
    case     10045:  pszName = LPSTR("WSAEOPNOTSUPP     "); break;
    case     10046:  pszName = LPSTR("WSAEPFNOSUPPORT   "); break;
    case     10047:  pszName = LPSTR("WSAEAFNOSUPPORT   "); break;
    case     10048:  pszName = LPSTR("WSAEADDRINUSE     "); break;
    case     10049:  pszName = LPSTR("WSAEADDRNOTAVAIL  "); break;
    case     10050:  pszName = LPSTR("WSAENETDOWN       "); break;
    case     10051:  pszName = LPSTR("WSAENETUNREACH    "); break;
    case     10052:  pszName = LPSTR("WSAENETRESET      "); break;
    case     10053:  pszName = LPSTR("WSAECONNABORTED   "); break;
    case     10054:  pszName = LPSTR("WSAECONNRESET     "); break;
    case     10055:  pszName = LPSTR("WSAENOBUFS        "); break;
    case     10056:  pszName = LPSTR("WSAEISCONN        "); break;
    case     10057:  pszName = LPSTR("WSAENOTCONN       "); break;
    case     10058:  pszName = LPSTR("WSAESHUTDOWN      "); break;
    case     10059:  pszName = LPSTR("WSAETOOMANYREFS   "); break;
    case     10060:  pszName = LPSTR("WSAETIMEDOUT      "); break;
    case     10061:  pszName = LPSTR("WSAECONNREFUSED   "); break;
    case     10062:  pszName = LPSTR("WSAELOOP          "); break;
    case     10063:  pszName = LPSTR("WSAENAMETOOLONG   "); break;
    case     10064:  pszName = LPSTR("WSAEHOSTDOWN      "); break;
    case     10065:  pszName = LPSTR("WSAEHOSTUNREACH   "); break;
    case     10066:  pszName = LPSTR("WSAENOTEMPTY      "); break;
    case     10067:  pszName = LPSTR("WSAEPROCLIM       "); break;
    case     10068:  pszName = LPSTR("WSAEUSERS         "); break;
    case     10069:  pszName = LPSTR("WSAEDQUOT         "); break;
    case     10070:  pszName = LPSTR("WSAESTALE         "); break;
    case     10071:  pszName = LPSTR("WSAEREMOTE        "); break;
    case     10091:  pszName = LPSTR("WSASYSNOTREADY    "); break;
    case  10092:  pszName = LPSTR("WSAVERNOTSUPPORTED"); break;
    case     10093:  pszName = LPSTR("WSANOTINITIALISED "); break;
    case     11001:  pszName = LPSTR("WSAHOST_NOT_FOUND "); break;
    case     11002:  pszName = LPSTR("WSATRY_AGAIN      "); break;
    case     11003:  pszName = LPSTR("WSANO_RECOVERY    "); break;
    case     11004:  pszName = LPSTR("WSANO_DATA        "); break;
    }
    printf("Error 0x%x (%s)\n", ErrCode, pszName);
}

/*****************************************************************************/
static void DisplaySECError(DWORD ErrCode)
{
    LPSTR pszName = NULL; // WinError.h

    switch (ErrCode)
    {
    case     SEC_E_BUFFER_TOO_SMALL:
        pszName = LPSTR("SEC_E_BUFFER_TOO_SMALL - The message buffer is too small. Used with the Digest SSP.");
        break;

    case     SEC_E_CRYPTO_SYSTEM_INVALID:
        pszName = LPSTR("SEC_E_CRYPTO_SYSTEM_INVALID - The cipher chosen for the security context is not supported. Used with the Digest SSP.");
        break;
    case     SEC_E_INCOMPLETE_MESSAGE:
        pszName = LPSTR("SEC_E_INCOMPLETE_MESSAGE - The data in the input buffer is incomplete. The application needs to read more data from the server and call DecryptMessage (General) again.");
        break;

    case     SEC_E_INVALID_HANDLE:
        pszName = LPSTR("SEC_E_INVALID_HANDLE - A context handle that is not valid was specified in the phContext parameter. Used with the Digest and Schannel SSPs.");
        break;

    case     SEC_E_INVALID_TOKEN:
        pszName = LPSTR("SEC_E_INVALID_TOKEN - The buffers are of the wrong type or no buffer of type SECBUFFER_DATA was found. Used with the Schannel SSP.");
        break;

    case     SEC_E_MESSAGE_ALTERED:
        pszName = LPSTR("SEC_E_MESSAGE_ALTERED - The message has been altered. Used with the Digest and Schannel SSPs.");
        break;

    case     SEC_E_OUT_OF_SEQUENCE:
        pszName = LPSTR("SEC_E_OUT_OF_SEQUENCE - The message was not received in the correct sequence.");
        break;

    case     SEC_E_QOP_NOT_SUPPORTED:
        pszName = LPSTR("SEC_E_QOP_NOT_SUPPORTED - Neither confidentiality nor integrity are supported by the security context. Used with the Digest SSP.");
        break;

    case     SEC_I_CONTEXT_EXPIRED:
        pszName = LPSTR("SEC_I_CONTEXT_EXPIRED - The message sender has finished using the connection and has initiated a shutdown.");
        break;

    case     SEC_I_RENEGOTIATE:
        pszName = LPSTR("SEC_I_RENEGOTIATE - The remote party requires a new handshake sequence or the application has just initiated a shutdown.");
        break;

    case     SEC_E_ENCRYPT_FAILURE:
        pszName = LPSTR("SEC_E_ENCRYPT_FAILURE - The specified data could not be encrypted.");
        break;

    case     SEC_E_DECRYPT_FAILURE:
        pszName = LPSTR("SEC_E_DECRYPT_FAILURE - The specified data could not be decrypted.");
        break;

    }
    printf("Error 0x%x %s \n", ErrCode, pszName);
}



/*****************************************************************************/
static void DisplayCertChain(PCCERT_CONTEXT  pServerCert, BOOL fLocal)
{
    CHAR szName[1000];
    PCCERT_CONTEXT pCurrentCert, pIssuerCert;
    DWORD dwVerificationFlags;

    printf("\n");

    // display leaf name
    if (!CertNameToStrA(pServerCert->dwCertEncodingType,
        &pServerCert->pCertInfo->Subject,
        CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
        szName, sizeof(szName)))
    {
        printf("**** Error 0x%x building subject name\n", GetLastError());
    }

    if (fLocal) printf("Client subject: %s\n", szName);
    else printf("Server subject: %s\n", szName);

    if (!CertNameToStrA(pServerCert->dwCertEncodingType,
        &pServerCert->pCertInfo->Issuer,
        CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
        szName, sizeof(szName)))
    {
        printf("**** Error 0x%x building issuer name\n", GetLastError());
    }

    if (fLocal) printf("Client issuer: %s\n", szName);
    else printf("Server issuer: %s\n\n", szName);


    // display certificate chain
    pCurrentCert = pServerCert;
    while (pCurrentCert != NULL)
    {
        dwVerificationFlags = 0;
        pIssuerCert = CertGetIssuerCertificateFromStore(pServerCert->hCertStore, pCurrentCert, NULL, &dwVerificationFlags);
        if (pIssuerCert == NULL)
        {
            if (pCurrentCert != pServerCert) CertFreeCertificateContext(pCurrentCert);
            break;
        }

        if (!CertNameToStrA(pIssuerCert->dwCertEncodingType,
            &pIssuerCert->pCertInfo->Subject,
            CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
            szName, sizeof(szName)))
        {
            printf("**** Error 0x%x building subject name\n", GetLastError());
        }

        printf("CA subject: %s\n", szName);

        if (!CertNameToStrA(pIssuerCert->dwCertEncodingType,
            &pIssuerCert->pCertInfo->Issuer,
            CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
            szName, sizeof(szName)))
        {
            printf("**** Error 0x%x building issuer name\n", GetLastError());
        }

        printf("CA issuer: %s\n\n", szName);

        if (pCurrentCert != pServerCert) CertFreeCertificateContext(pCurrentCert);
        pCurrentCert = pIssuerCert;
        pIssuerCert = NULL;
    }
}

/*****************************************************************************/
static void DisplayConnectionInfo(CtxtHandle* phContext)
{

    SECURITY_STATUS Status;
    SecPkgContext_ConnectionInfo ConnectionInfo;

    Status = g_pSSPI->QueryContextAttributes(phContext, SECPKG_ATTR_CONNECTION_INFO, (PVOID)&ConnectionInfo);
    if (Status != SEC_E_OK) { printf("Error 0x%x querying connection info\n", Status); return; }

    printf("\n");

    switch (ConnectionInfo.dwProtocol)
    {
    case SP_PROT_TLS1_CLIENT:
        printf("Protocol: TLS1\n");
        break;

    case SP_PROT_SSL3_CLIENT:
        printf("Protocol: SSL3\n");
        break;

    case SP_PROT_PCT1_CLIENT:
        printf("Protocol: PCT\n");
        break;

    case SP_PROT_SSL2_CLIENT:
        printf("Protocol: SSL2\n");
        break;

    default:
        printf("Protocol: 0x%x\n", ConnectionInfo.dwProtocol);
    }

    switch (ConnectionInfo.aiCipher)
    {
    case CALG_RC4:
        printf("Cipher: RC4\n");
        break;

    case CALG_3DES:
        printf("Cipher: Triple DES\n");
        break;

    case CALG_RC2:
        printf("Cipher: RC2\n");
        break;

    case CALG_DES:
    case CALG_CYLINK_MEK:
        printf("Cipher: DES\n");
        break;

    case CALG_SKIPJACK:
        printf("Cipher: Skipjack\n");
        break;

    default:
        printf("Cipher: 0x%x\n", ConnectionInfo.aiCipher);
    }

    printf("Cipher strength: %d\n", ConnectionInfo.dwCipherStrength);

    switch (ConnectionInfo.aiHash)
    {
    case CALG_MD5:
        printf("Hash: MD5\n");
        break;

    case CALG_SHA:
        printf("Hash: SHA\n");
        break;

    default:
        printf("Hash: 0x%x\n", ConnectionInfo.aiHash);
    }

    printf("Hash strength: %d\n", ConnectionInfo.dwHashStrength);

    switch (ConnectionInfo.aiExch)
    {
    case CALG_RSA_KEYX:
    case CALG_RSA_SIGN:
        printf("Key exchange: RSA\n");
        break;

    case CALG_KEA_KEYX:
        printf("Key exchange: KEA\n");
        break;

    case CALG_DH_EPHEM:
        printf("Key exchange: DH Ephemeral\n");
        break;

    default:
        printf("Key exchange: 0x%x\n", ConnectionInfo.aiExch);
    }

    printf("Key exchange strength: %d\n", ConnectionInfo.dwExchStrength);
}


/*****************************************************************************/
static void PrintHexDump(DWORD length, PBYTE buffer)
{
    DWORD i, count, index;
    CHAR rgbDigits[] = "0123456789abcdef";
    CHAR rgbLine[100];
    char cbLine;

    for (index = 0; length; length -= count, buffer += count, index += count)
    {
        count = (length > 16) ? 16 : length;
        sprintf(rgbLine, "%4.4x  ", index);
        cbLine = 6;

        for (i = 0; i < count; i++)
        {
            rgbLine[cbLine++] = rgbDigits[buffer[i] >> 4];
            rgbLine[cbLine++] = rgbDigits[buffer[i] & 0x0f];
            if (i == 7) rgbLine[cbLine++] = ':';
            else rgbLine[cbLine++] = ' ';
        }
        for (; i < 16; i++)
        {
            rgbLine[cbLine++] = ' ';
            rgbLine[cbLine++] = ' ';
            rgbLine[cbLine++] = ' ';
        }
        rgbLine[cbLine++] = ' ';

        for (i = 0; i < count; i++)
        {
            if (buffer[i] < 32 || buffer[i] > 126 || buffer[i] == '%') rgbLine[cbLine++] = '.';
            else rgbLine[cbLine++] = buffer[i];
        }
        rgbLine[cbLine++] = 0;
        printf("%s\n", rgbLine);
    }
}

/*****************************************************************************/
static void PrintText(DWORD length, PBYTE buffer) // handle unprintable charaters
{
    int i; //

    printf("\n"); // "length = %d bytes \n", length);
    for (i = 0; i < (int)length; i++)
    {
        if (buffer[i] == 10 || buffer[i] == 13)
            printf("%c", (char)buffer[i]);
        else if (buffer[i] < 32 || buffer[i] > 126 || buffer[i] == '%')
            printf("%c", '.');
        else
            printf("%c", (char)buffer[i]);
    }
    printf("\n");
}



/*****************************************************************************/
static void WriteDataToFile(PSTR pszData, PBYTE pbData, DWORD cbData)
{
    FILE* file;

    file = fopen(pszData, "wb");
    if (file == NULL)
    {
        printf("**** Error opening file '%s'\n", pszData); return;
    }

    if (fwrite(pbData, 1, cbData, file) != cbData)
    {
        printf("**** Error writing to file\n"); return;
    }

    fclose(file);
}





/*****************************************************************************/
BOOL LoadSecurityLibrary(void) // load SSPI.DLL, set up a special table - PSecurityFunctionTable
{
    INIT_SECURITY_INTERFACE pInitSecurityInterface;
    //  QUERY_CREDENTIALS_ATTRIBUTES_FN pQueryCredentialsAttributes;
    OSVERSIONINFO VerInfo;
    UCHAR lpszDLL[MAX_PATH];


    //  Find out which security DLL to use, depending on
    //  whether we are on Win2K, NT or Win9x
    VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&VerInfo)) return FALSE;

    if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && VerInfo.dwMajorVersion == 4)
    {
        strcpy(
            (char*)lpszDLL, 
            "Security.dll"
            //NT4_DLL_NAME
        ); // NT4_DLL_NAME TEXT("Security.dll")
    }
    else if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ||
        VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        strcpy(
            (char*)lpszDLL, 
            "Secur32.dll"
            //DLL_NAME
        ); // DLL_NAME TEXT("Secur32.dll")
    }
    else
    {
        printf("System not recognized\n"); return FALSE;
    }


    //  Load Security DLL
    g_hSecurity = LoadLibraryA((LPCSTR)lpszDLL);
    if (g_hSecurity == NULL) { printf("Error 0x%x loading %s.\n", GetLastError(), lpszDLL); return FALSE; }

    pInitSecurityInterface = (INIT_SECURITY_INTERFACE)GetProcAddress(g_hSecurity, "InitSecurityInterfaceA");
    if (pInitSecurityInterface == NULL) { printf("Error 0x%x reading InitSecurityInterface entry point.\n", GetLastError()); return FALSE; }

    g_pSSPI = pInitSecurityInterface(); // call InitSecurityInterfaceA(void);
    if (g_pSSPI == NULL) { printf("Error 0x%x reading security interface.\n", GetLastError()); return FALSE; }

    return TRUE; // and PSecurityFunctionTable
}


/*****************************************************************************/
void UnloadSecurityLibrary(void)
{
    FreeLibrary(g_hSecurity);
    g_hSecurity = NULL;
}


/*****************************************************************************/
static DWORD VerifyServerCertificate(PCCERT_CONTEXT pServerCert, PSTR pszServerName, DWORD dwCertFlags)
{
    HTTPSPolicyCallbackData  polHttps;
    CERT_CHAIN_POLICY_PARA   PolicyPara;
    CERT_CHAIN_POLICY_STATUS PolicyStatus;
    CERT_CHAIN_PARA          ChainPara;
    PCCERT_CHAIN_CONTEXT     pChainContext = NULL;
    DWORD                                         cchServerName, Status;
    LPSTR rgszUsages[] = { LPSTR(szOID_PKIX_KP_SERVER_AUTH),
                               LPSTR(szOID_SERVER_GATED_CRYPTO),
                               LPSTR(szOID_SGC_NETSCAPE) };

    DWORD cUsages = sizeof(rgszUsages) / sizeof(LPSTR);

    PWSTR   pwszServerName = NULL;


    if (pServerCert == NULL)
    {
        Status = SEC_E_WRONG_PRINCIPAL; goto cleanup;
    }

    // Convert server name to unicode.
    if (pszServerName == NULL || strlen(pszServerName) == 0)
    {
        Status = SEC_E_WRONG_PRINCIPAL; goto cleanup;
    }

    cchServerName = MultiByteToWideChar(CP_ACP, 0, pszServerName, -1, NULL, 0);
    pwszServerName = (PWSTR) LocalAlloc(LMEM_FIXED, cchServerName * sizeof(WCHAR));
    if (pwszServerName == NULL)
    {
        Status = SEC_E_INSUFFICIENT_MEMORY; goto cleanup;
    }

    cchServerName = MultiByteToWideChar(CP_ACP, 0, pszServerName, -1, pwszServerName, cchServerName);
    if (cchServerName == 0)
    {
        Status = SEC_E_WRONG_PRINCIPAL; goto cleanup;
    }


    // Build certificate chain.
    ZeroMemory(&ChainPara, sizeof(ChainPara));
    ChainPara.cbSize = sizeof(ChainPara);
    ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_OR;
    ChainPara.RequestedUsage.Usage.cUsageIdentifier = cUsages;
    ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = rgszUsages;

    if (!CertGetCertificateChain(NULL,
        pServerCert,
        NULL,
        pServerCert->hCertStore,
        &ChainPara,
        0,
        NULL,
        &pChainContext))
    {
        Status = GetLastError();
        printf("Error 0x%x returned by CertGetCertificateChain!\n", Status);
        goto cleanup;
    }


    // Validate certificate chain.
    ZeroMemory(&polHttps, sizeof(HTTPSPolicyCallbackData));
    polHttps.cbStruct = sizeof(HTTPSPolicyCallbackData);
    polHttps.dwAuthType = AUTHTYPE_SERVER;
    polHttps.fdwChecks = dwCertFlags;
    polHttps.pwszServerName = pwszServerName;

    memset(&PolicyPara, 0, sizeof(PolicyPara));
    PolicyPara.cbSize = sizeof(PolicyPara);
    PolicyPara.pvExtraPolicyPara = &polHttps;

    memset(&PolicyStatus, 0, sizeof(PolicyStatus));
    PolicyStatus.cbSize = sizeof(PolicyStatus);

    if (!CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL,
        pChainContext,
        &PolicyPara,
        &PolicyStatus))
    {
        Status = GetLastError();
        printf("Error 0x%x returned by CertVerifyCertificateChainPolicy!\n", Status);
        goto cleanup;
    }

    if (PolicyStatus.dwError)
    {
        Status = PolicyStatus.dwError;
        DisplayWinVerifyTrustError(Status);
        goto cleanup;
    }

    Status = SEC_E_OK;


cleanup:
    if (pChainContext)  CertFreeCertificateChain(pChainContext);
    if (pwszServerName) LocalFree(pwszServerName);

    return Status;
}


/*****************************************************************************/
static SECURITY_STATUS CreateCredentials(LPSTR pszUser, PCredHandle phCreds)
{
    // Open the "MY" certificate store, where IE stores client certificates.
    // Windows maintains 4 stores -- MY, CA, ROOT, SPC.
    if (hMyCertStore == NULL)
    {
        hMyCertStore = CertOpenSystemStoreA(0, "MY");
        if (!hMyCertStore)
        {
            printf("**** Error 0x%x returned by CertOpenSystemStore\n", GetLastError());
            return SEC_E_NO_CREDENTIALS;
        }
    }


    // If a user name is specified, then attempt to find a client
    // certificate. Otherwise, just create a NULL credential.
    PCCERT_CONTEXT   pCertContext = NULL;
    if (pszUser)
    {
        // Find client certificate. Note that this sample just searches for a
        // certificate that contains the user name somewhere in the subject name.
        // A real application should be a bit less casual.
        pCertContext = CertFindCertificateInStore(
            hMyCertStore,                     // hCertStore
            X509_ASN_ENCODING,             // dwCertEncodingType
            0,                                             // dwFindFlags
            CERT_FIND_SUBJECT_STR_A,// dwFindType
            pszUser,                         // *pvFindPara
            NULL
        );                                 // pPrevCertContext


        if (pCertContext == NULL)
        {
            printf("**** Error 0x%x returned by CertFindCertificateInStore\n", GetLastError());
            if (GetLastError() == CRYPT_E_NOT_FOUND) 
                printf("CRYPT_E_NOT_FOUND - property doesn't exist\n");
            return SEC_E_NO_CREDENTIALS;
        }
    }


    // Build Schannel credential structure. Currently, this sample only
    // specifies the protocol to be used (and optionally the certificate,
    // of course). Real applications may wish to specify other parameters as well.
    ZeroMemory(&SchannelCred, sizeof(SchannelCred));

    SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;
    if (pCertContext)
    {
        SchannelCred.cCreds = 1;
        SchannelCred.paCred = &pCertContext;
    }

    SchannelCred.grbitEnabledProtocols = dwProtocol;

    DWORD            cSupportedAlgs = 0;
    ALG_ID           rgbSupportedAlgs[16];
    if (aiKeyExch) 
        rgbSupportedAlgs[cSupportedAlgs++] = aiKeyExch;

    if (cSupportedAlgs)
    {
        SchannelCred.cSupportedAlgs = cSupportedAlgs;
        SchannelCred.palgSupportedAlgs = rgbSupportedAlgs;
    }

    SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS;

    // The SCH_CRED_MANUAL_CRED_VALIDATION flag is specified because
    // this sample verifies the server certificate manually.
    // Applications that expect to run on WinNT, Win9x, or WinME
    // should specify this flag and also manually verify the server
    // certificate. Applications running on newer versions of Windows can
    // leave off this flag, in which case the InitializeSecurityContext
    // function will validate the server certificate automatically.
    SchannelCred.dwFlags |= SCH_CRED_MANUAL_CRED_VALIDATION;


    SECURITY_STATUS  Status;
    TimeStamp        tsExpiry;
    // Create an SSPI credential.
    Status = AcquireCredentialsHandleA(
        NULL,                 // Name of principal    
        (LPSTR)UNISP_NAME_A,         // Name of package
        SECPKG_CRED_OUTBOUND, // Flags indicating use
        NULL,                 // Pointer to logon ID
        &SchannelCred,        // Package specific data
        NULL,                 // Pointer to GetKey() func
        NULL,                 // Value to pass to GetKey()
        phCreds,              // (out) Cred Handle
        &tsExpiry);          // (out) Lifetime (optional)

    if (Status != SEC_E_OK) 
        printf("**** Error 0x%x returned by AcquireCredentialsHandle\n", Status);

    // cleanup: Free the certificate context. Schannel has already made its own copy.
    if (pCertContext) 
        CertFreeCertificateContext(pCertContext);

    return Status;
}

/*****************************************************************************/
static INT ConnectToServer(LPSTR pszServerName, INT iPortNumber, SOCKET* pSocket)
{ //                                    in                in                 out
    SOCKET Socket;
    struct sockaddr_in sin;
    struct hostent* hp;


    Socket = socket(PF_INET, SOCK_STREAM, 0);
    if (Socket == INVALID_SOCKET)
    {
        printf("**** Error %d creating socket\n", WSAGetLastError());
        DisplayWinSockError(WSAGetLastError());
        return WSAGetLastError();
    }


    if (fUseProxy)
    {
        sin.sin_family = AF_INET;
        sin.sin_port = ntohs((u_short)iProxyPort);
        if ((hp = gethostbyname(pszProxyServer)) == NULL)
        {
            printf("**** Error %d returned by gethostbyname using Proxy\n", WSAGetLastError());
            DisplayWinSockError(WSAGetLastError());
            return WSAGetLastError();
        }
        else
            memcpy(&sin.sin_addr, hp->h_addr, 4);
    }

    else // No proxy used
    {
        sin.sin_family = AF_INET;
        sin.sin_port = htons((u_short)iPortNumber);
        if ((hp = gethostbyname(pszServerName)) == NULL)
        {
            printf("**** Error returned by gethostbyname\n");
            DisplayWinSockError(WSAGetLastError());
            return WSAGetLastError();
        }
        else
            memcpy(&sin.sin_addr, hp->h_addr, 4);
    }


    if (connect(Socket, (struct sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
    {
        printf("**** Error %d connecting to \"%s\" (%s)\n", WSAGetLastError(), pszServerName, inet_ntoa(sin.sin_addr));
        closesocket(Socket);
        DisplayWinSockError(WSAGetLastError());
        return WSAGetLastError();
    }


    if (fUseProxy)
    {
        BYTE  pbMessage[200];
        DWORD cbMessage;

        // Build message for proxy server
        strcpy((char*)pbMessage, "CONNECT ");
        strcat((char*)pbMessage, pszServerName);
        strcat((char*)pbMessage, ":");
        _itoa(iPortNumber, (char*)pbMessage + strlen((char*)pbMessage), 10);
        strcat((char*)pbMessage, " HTTP/1.0\r\nUser-Agent: webclient\r\n\r\n");
        cbMessage = (DWORD)strlen((char*)pbMessage);

        // Send message to proxy server
        if (send(Socket, (char*)pbMessage, cbMessage, 0) == SOCKET_ERROR)
        {
            printf("**** Error %d sending message to proxy!\n", WSAGetLastError());
            DisplayWinSockError(WSAGetLastError());
            return WSAGetLastError();
        }

        // Receive message from proxy server
        cbMessage = recv(Socket, (char*)pbMessage, 200, 0);
        if (cbMessage == SOCKET_ERROR)
        {
            printf("**** Error %d receiving message from proxy\n", WSAGetLastError());
            DisplayWinSockError(WSAGetLastError());
            return WSAGetLastError();
        }
        // this sample is limited but in normal use it
        // should continue to receive until CR LF CR LF is received
    }
    *pSocket = Socket;

    return SEC_E_OK;
}

/*****************************************************************************/
static LONG DisconnectFromServer(SOCKET Socket, PCredHandle phCreds, CtxtHandle* phContext)
{
    PBYTE                    pbMessage;
    DWORD                    dwType, dwSSPIFlags, dwSSPIOutFlags, cbMessage, cbData, Status;
    SecBufferDesc OutBuffer;
    SecBuffer     OutBuffers[1];
    TimeStamp     tsExpiry;


    dwType = SCHANNEL_SHUTDOWN; // Notify schannel that we are about to close the connection.

    OutBuffers[0].pvBuffer = &dwType;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer = sizeof(dwType);

    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    Status = g_pSSPI->ApplyControlToken(phContext, &OutBuffer);
    if (FAILED(Status)) { printf("**** Error 0x%x returned by ApplyControlToken\n", Status); goto cleanup; }


    // Build an SSL close notify message.
    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT |
        ISC_REQ_REPLAY_DETECT |
        ISC_REQ_CONFIDENTIALITY |
        ISC_RET_EXTENDED_ERROR |
        ISC_REQ_ALLOCATE_MEMORY |
        ISC_REQ_STREAM;

    OutBuffers[0].pvBuffer = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer = 0;

    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    Status = InitializeSecurityContextA(phCreds,
        phContext,
        NULL,
        dwSSPIFlags,
        0,
        SECURITY_NATIVE_DREP,
        NULL,
        0,
        phContext,
        &OutBuffer,
        &dwSSPIOutFlags,
        &tsExpiry);

    if (FAILED(Status)) { printf("**** Error 0x%x returned by InitializeSecurityContext\n", Status); goto cleanup; }

    pbMessage = (PBYTE)OutBuffers[0].pvBuffer;
    cbMessage = OutBuffers[0].cbBuffer;


    // Send the close notify message to the server.
    if (pbMessage != NULL && cbMessage != 0)
    {
        cbData = send(Socket, (char*)pbMessage, cbMessage, 0);
        if (cbData == SOCKET_ERROR || cbData == 0)
        {
            Status = WSAGetLastError();
            printf("**** Error %d sending close notify\n", Status);
            DisplayWinSockError(WSAGetLastError());
            goto cleanup;
        }
        printf("Sending Close Notify\n");
        printf("%d bytes of handshake data sent\n", cbData);
        if (fVerbose) { PrintHexDump(cbData, pbMessage); printf("\n"); }
        g_pSSPI->FreeContextBuffer(pbMessage); // Free output buffer.
    }


cleanup:
    g_pSSPI->DeleteSecurityContext(phContext); // Free the security context.
    closesocket(Socket); // Close the socket.

    return Status;
}



/*****************************************************************************/
static void GetNewClientCredentials(CredHandle* phCreds, CtxtHandle* phContext)
{
    SecPkgContext_IssuerListInfoEx    IssuerListInfo;
    // Read list of trusted issuers from schannel.
    SECURITY_STATUS Status = g_pSSPI->QueryContextAttributes(
        phContext, 
        SECPKG_ATTR_ISSUER_LIST_EX, 
        (PVOID)&IssuerListInfo
    );
    if (Status != SEC_E_OK) 
    { 
        printf("Error 0x%x querying issuer list info\n", Status); 
        return; 
    }

    // Enumerate the client certificates.
    CERT_CHAIN_FIND_BY_ISSUER_PARA    FindByIssuerPara;
    ZeroMemory(&FindByIssuerPara, sizeof(FindByIssuerPara));
    FindByIssuerPara.cbSize = sizeof(FindByIssuerPara);
    FindByIssuerPara.pszUsageIdentifier = szOID_PKIX_KP_CLIENT_AUTH;
    FindByIssuerPara.dwKeySpec = 0;
    FindByIssuerPara.cIssuer = IssuerListInfo.cIssuers;
    FindByIssuerPara.rgIssuer = IssuerListInfo.aIssuers;

    PCCERT_CHAIN_CONTEXT pChainContext = NULL;
    PCCERT_CONTEXT pCertContext;
    CredHandle hCreds;
    TimeStamp tsExpiry;
    while (true)
    {   // Find a certificate chain.
        pChainContext = CertFindChainInStore(
            hMyCertStore,
            X509_ASN_ENCODING,
            0,
            CERT_CHAIN_FIND_BY_ISSUER,
            &FindByIssuerPara,
            pChainContext
        );
        if (pChainContext == NULL) 
        { 
            printf("Error 0x%x finding cert chain\n", GetLastError()); 
            break; 
        }

        printf("\ncertificate chain found\n");

        // Get pointer to leaf certificate context.
        pCertContext = pChainContext->rgpChain[0]->rgpElement[0]->pCertContext;

        // Create schannel credential.
        SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;
        SchannelCred.cCreds = 1;
        SchannelCred.paCred = &pCertContext;

        Status = AcquireCredentialsHandleA(
            NULL,                   // Name of principal
            (LPSTR)UNISP_NAME_A,    // Name of package
            SECPKG_CRED_OUTBOUND,   // Flags indicating use
            NULL,                   // Pointer to logon ID
            &SchannelCred,          // Package specific data
            NULL,                   // Pointer to GetKey() func
            NULL,                   // Value to pass to GetKey()
            &hCreds,                // (out) Cred Handle
            &tsExpiry               // (out) Lifetime (optional)
        );            

        if (Status != SEC_E_OK) 
        { 
            printf("**** Error 0x%x returned by AcquireCredentialsHandle\n", Status); 
            continue; 
        }

        printf("\nnew schannel credential created\n");

        g_pSSPI->FreeCredentialsHandle(phCreds); // Destroy the old credentials.

        *phCreds = hCreds;

    }
}

/*****************************************************************************/
static SECURITY_STATUS ClientHandshakeLoop(SOCKET          Socket,         // in
    PCredHandle     phCreds,        // in
    CtxtHandle* phContext,      // in, out
    BOOL            fDoInitialRead, // in
    SecBuffer* pExtraData)    // out

{

    SecBufferDesc   OutBuffer, InBuffer;
    SecBuffer       InBuffers[2], OutBuffers[1];
    DWORD           dwSSPIFlags, dwSSPIOutFlags, cbData, cbIoBuffer;
    TimeStamp       tsExpiry;
    SECURITY_STATUS scRet;
    PUCHAR          IoBuffer;
    BOOL            fDoRead;


    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY |
        ISC_RET_EXTENDED_ERROR | ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM;


    // Allocate data buffer.
    IoBuffer = (PUCHAR)LocalAlloc(LMEM_FIXED, IO_BUFFER_SIZE);
    if (IoBuffer == NULL) { printf("**** Out of memory (1)\n"); return SEC_E_INTERNAL_ERROR; }
    cbIoBuffer = 0;
    fDoRead = fDoInitialRead;



    // Loop until the handshake is finished or an error occurs.
    scRet = SEC_I_CONTINUE_NEEDED;

    while (scRet == SEC_I_CONTINUE_NEEDED ||
        scRet == SEC_E_INCOMPLETE_MESSAGE ||
        scRet == SEC_I_INCOMPLETE_CREDENTIALS)
    {
        if (0 == cbIoBuffer || scRet == SEC_E_INCOMPLETE_MESSAGE) // Read data from server.
        {
            if (fDoRead)
            {
                cbData = recv(Socket, (char*)IoBuffer + cbIoBuffer, IO_BUFFER_SIZE - cbIoBuffer, 0);
                if (cbData == SOCKET_ERROR)
                {
                    printf("**** Error %d reading data from server\n", WSAGetLastError());
                    scRet = SEC_E_INTERNAL_ERROR;
                    break;
                }
                else if (cbData == 0)
                {
                    printf("**** Server unexpectedly disconnected\n");
                    scRet = SEC_E_INTERNAL_ERROR;
                    break;
                }
                printf("%d bytes of handshake data received\n", cbData);
                if (fVerbose) { PrintHexDump(cbData, IoBuffer + cbIoBuffer); printf("\n"); }
                cbIoBuffer += cbData;
            }
            else
                fDoRead = TRUE;
        }



        // Set up the input buffers. Buffer 0 is used to pass in data
        // received from the server. Schannel will consume some or all
        // of this. Leftover data (if any) will be placed in buffer 1 and
        // given a buffer type of SECBUFFER_EXTRA.
        InBuffers[0].pvBuffer = IoBuffer;
        InBuffers[0].cbBuffer = cbIoBuffer;
        InBuffers[0].BufferType = SECBUFFER_TOKEN;

        InBuffers[1].pvBuffer = NULL;
        InBuffers[1].cbBuffer = 0;
        InBuffers[1].BufferType = SECBUFFER_EMPTY;

        InBuffer.cBuffers = 2;
        InBuffer.pBuffers = InBuffers;
        InBuffer.ulVersion = SECBUFFER_VERSION;


        // Set up the output buffers. These are initialized to NULL
        // so as to make it less likely we'll attempt to free random
        // garbage later.
        OutBuffers[0].pvBuffer = NULL;
        OutBuffers[0].BufferType = SECBUFFER_TOKEN;
        OutBuffers[0].cbBuffer = 0;

        OutBuffer.cBuffers = 1;
        OutBuffer.pBuffers = OutBuffers;
        OutBuffer.ulVersion = SECBUFFER_VERSION;


        // Call InitializeSecurityContext.
        scRet = InitializeSecurityContextA(phCreds,
            phContext,
            NULL,
            dwSSPIFlags,
            0,
            SECURITY_NATIVE_DREP,
            &InBuffer,
            0,
            NULL,
            &OutBuffer,
            &dwSSPIOutFlags,
            &tsExpiry);


        // If InitializeSecurityContext was successful (or if the error was
        // one of the special extended ones), send the contends of the output
        // buffer to the server.
        if (scRet == SEC_E_OK ||
            scRet == SEC_I_CONTINUE_NEEDED ||
            FAILED(scRet) && (dwSSPIOutFlags & ISC_RET_EXTENDED_ERROR))
        {
            if (OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL)
            {
                cbData = send(Socket, (char*)OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer, 0);
                if (cbData == SOCKET_ERROR || cbData == 0)
                {
                    printf("**** Error %d sending data to server (2)\n", WSAGetLastError());
                    DisplayWinSockError(WSAGetLastError());
                    g_pSSPI->FreeContextBuffer(OutBuffers[0].pvBuffer);
                    g_pSSPI->DeleteSecurityContext(phContext);
                    return SEC_E_INTERNAL_ERROR;
                }
                printf("%d bytes of handshake data sent\n", cbData);
                if (fVerbose) { PrintHexDump(cbData, (PBYTE)OutBuffers[0].pvBuffer); printf("\n"); }

                // Free output buffer.
                g_pSSPI->FreeContextBuffer(OutBuffers[0].pvBuffer);
                OutBuffers[0].pvBuffer = NULL;
            }
        }



        // If InitializeSecurityContext returned SEC_E_INCOMPLETE_MESSAGE,
        // then we need to read more data from the server and try again.
        if (scRet == SEC_E_INCOMPLETE_MESSAGE) continue;


        // If InitializeSecurityContext returned SEC_E_OK, then the
        // handshake completed successfully.
        if (scRet == SEC_E_OK)
        {
            // If the "extra" buffer contains data, this is encrypted application
            // protocol layer stuff. It needs to be saved. The application layer
            // will later decrypt it with DecryptMessage.
            printf("Handshake was successful\n");

            if (InBuffers[1].BufferType == SECBUFFER_EXTRA)
            {
                pExtraData->pvBuffer = LocalAlloc(LMEM_FIXED, InBuffers[1].cbBuffer);
                if (pExtraData->pvBuffer == NULL) { printf("**** Out of memory (2)\n"); return SEC_E_INTERNAL_ERROR; }

                MoveMemory(pExtraData->pvBuffer,
                    IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer),
                    InBuffers[1].cbBuffer);

                pExtraData->cbBuffer = InBuffers[1].cbBuffer;
                pExtraData->BufferType = SECBUFFER_TOKEN;

                printf("%d bytes of app data was bundled with handshake data\n", pExtraData->cbBuffer);
            }
            else
            {
                pExtraData->pvBuffer = NULL;
                pExtraData->cbBuffer = 0;
                pExtraData->BufferType = SECBUFFER_EMPTY;
            }
            break; // Bail out to quit
        }



        // Check for fatal error.
        if (FAILED(scRet)) { printf("**** Error 0x%x returned by InitializeSecurityContext (2)\n", scRet); break; }

        // If InitializeSecurityContext returned SEC_I_INCOMPLETE_CREDENTIALS,
        // then the server just requested client authentication.
        if (scRet == SEC_I_INCOMPLETE_CREDENTIALS)
        {
            // Busted. The server has requested client authentication and
            // the credential we supplied didn't contain a client certificate.
            // This function will read the list of trusted certificate
            // authorities ("issuers") that was received from the server
            // and attempt to find a suitable client certificate that
            // was issued by one of these. If this function is successful,
            // then we will connect using the new certificate. Otherwise,
            // we will attempt to connect anonymously (using our current credentials).
            GetNewClientCredentials(phCreds, phContext);

            // Go around again.
            fDoRead = FALSE;
            scRet = SEC_I_CONTINUE_NEEDED;
            continue;
        }

        // Copy any leftover data from the "extra" buffer, and go around again.
        if (InBuffers[1].BufferType == SECBUFFER_EXTRA)
        {
            MoveMemory(IoBuffer, IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer), InBuffers[1].cbBuffer);
            cbIoBuffer = InBuffers[1].cbBuffer;
        }
        else
            cbIoBuffer = 0;
    }

    // Delete the security context in the case of a fatal error.
    if (FAILED(scRet)) g_pSSPI->DeleteSecurityContext(phContext);
    LocalFree(IoBuffer);

    return scRet;
}


/*****************************************************************************/
static SECURITY_STATUS PerformClientHandshake(SOCKET          Socket,        // in
    PCredHandle     phCreds,       // in
    LPSTR           pszServerName, // in
    CtxtHandle* phContext,     // out
    SecBuffer* pExtraData)   // out
{

    SecBufferDesc   OutBuffer;
    SecBuffer       OutBuffers[1];
    DWORD           dwSSPIFlags, dwSSPIOutFlags, cbData;
    TimeStamp       tsExpiry;
    SECURITY_STATUS scRet;


    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY |
        ISC_RET_EXTENDED_ERROR | ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_STREAM;


    //  Initiate a ClientHello message and generate a token.
    OutBuffers[0].pvBuffer = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer = 0;

    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    scRet = InitializeSecurityContextA(phCreds,
        NULL,
        pszServerName,
        dwSSPIFlags,
        0,
        SECURITY_NATIVE_DREP,
        NULL,
        0,
        phContext,
        &OutBuffer,
        &dwSSPIOutFlags,
        &tsExpiry);

    if (scRet != SEC_I_CONTINUE_NEEDED) { printf("**** Error %d returned by InitializeSecurityContext (1)\n", scRet); return scRet; }

    // Send response to server if there is one.
    if (OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL)
    {
        cbData = send(Socket, (char*)OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer, 0);
        if (cbData == SOCKET_ERROR || cbData == 0)
        {
            printf("**** Error %d sending data to server (1)\n", WSAGetLastError());
            g_pSSPI->FreeContextBuffer(OutBuffers[0].pvBuffer);
            g_pSSPI->DeleteSecurityContext(phContext);
            return SEC_E_INTERNAL_ERROR;
        }
        printf("%d bytes of handshake data sent\n", cbData);
        if (fVerbose) { PrintHexDump(cbData, (PBYTE)OutBuffers[0].pvBuffer); printf("\n"); }
        g_pSSPI->FreeContextBuffer(OutBuffers[0].pvBuffer); // Free output buffer.
        OutBuffers[0].pvBuffer = NULL;
    }

    return ClientHandshakeLoop(Socket, phCreds, phContext, TRUE, pExtraData);
}



/*****************************************************************************/
static DWORD EncryptSend(SOCKET Socket, CtxtHandle* phContext, PBYTE pbIoBuffer, SecPkgContext_StreamSizes Sizes)
// http://msdn.microsoft.com/en-us/library/aa375378(VS.85).aspx
// The encrypted message is encrypted in place, overwriting the original contents of its buffer.
{
    SECURITY_STATUS    scRet;            // unsigned long cbBuffer;    // Size of the buffer, in bytes
    SecBufferDesc        Message;        // unsigned long BufferType;  // Type of the buffer (below)
    SecBuffer                Buffers[4];    // void    SEC_FAR * pvBuffer;   // Pointer to the buffer
    DWORD                        cbMessage, cbData;
    PBYTE                        pbMessage;


    pbMessage = pbIoBuffer + Sizes.cbHeader; // Offset by "header size"
    cbMessage = (DWORD)strlen((char*)pbMessage);
    printf("Sending %d bytes of plaintext:", cbMessage); PrintText(cbMessage, pbMessage);
    if (fVerbose) { PrintHexDump(cbMessage, pbMessage); printf("\n"); }


    // Encrypt the HTTP request.
    Buffers[0].pvBuffer = pbIoBuffer;                                // Pointer to buffer 1
    Buffers[0].cbBuffer = Sizes.cbHeader;                        // length of header
    Buffers[0].BufferType = SECBUFFER_STREAM_HEADER;    // Type of the buffer

    Buffers[1].pvBuffer = pbMessage;                                // Pointer to buffer 2
    Buffers[1].cbBuffer = cbMessage;                                // length of the message
    Buffers[1].BufferType = SECBUFFER_DATA;                        // Type of the buffer

    Buffers[2].pvBuffer = pbMessage + cbMessage;        // Pointer to buffer 3
    Buffers[2].cbBuffer = Sizes.cbTrailer;                    // length of the trailor
    Buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;    // Type of the buffer

    Buffers[3].pvBuffer = SECBUFFER_EMPTY;                    // Pointer to buffer 4
    Buffers[3].cbBuffer = SECBUFFER_EMPTY;                    // length of buffer 4
    Buffers[3].BufferType = SECBUFFER_EMPTY;                    // Type of the buffer 4


    Message.ulVersion = SECBUFFER_VERSION;    // Version number
    Message.cBuffers = 4;                                    // Number of buffers - must contain four SecBuffer structures.
    Message.pBuffers = Buffers;                        // Pointer to array of buffers
    scRet = g_pSSPI->EncryptMessage(phContext, 0, &Message, 0); // must contain four SecBuffer structures.
    if (FAILED(scRet)) { printf("**** Error 0x%x returned by EncryptMessage\n", scRet); return scRet; }


    // Send the encrypted data to the server.                                            len                                                                         flags
    cbData = send(Socket, (char*)pbIoBuffer, Buffers[0].cbBuffer + Buffers[1].cbBuffer + Buffers[2].cbBuffer, 0);

    printf("%d bytes of encrypted data sent\n", cbData);
    if (fVerbose) { PrintHexDump(cbData, pbIoBuffer); printf("\n"); }

    return cbData; // send( Socket, pbIoBuffer,    Sizes.cbHeader + strlen(pbMessage) + Sizes.cbTrailer,  0 );

}


/*****************************************************************************/
static SECURITY_STATUS ReadDecrypt(SOCKET Socket, PCredHandle phCreds, CtxtHandle* phContext, PBYTE pbIoBuffer, DWORD    cbIoBufferLength)

// calls recv() - blocking socket read
// http://msdn.microsoft.com/en-us/library/ms740121(VS.85).aspx

// The encrypted message is decrypted in place, overwriting the original contents of its buffer.
// http://msdn.microsoft.com/en-us/library/aa375211(VS.85).aspx

{
    SecBuffer                ExtraBuffer;
    SecBuffer* pDataBuffer, * pExtraBuffer;

    SECURITY_STATUS    scRet;            // unsigned long cbBuffer;    // Size of the buffer, in bytes
    SecBufferDesc        Message;        // unsigned long BufferType;  // Type of the buffer (below)
    SecBuffer                Buffers[4];    // void    SEC_FAR * pvBuffer;   // Pointer to the buffer

    DWORD                        cbIoBuffer, cbData, length;
    PBYTE                        buff;
    int i;



    // Read data from server until done.
    cbIoBuffer = 0;
    scRet = 0;
    while (TRUE) // Read some data.
    {
        if (cbIoBuffer == 0 || scRet == SEC_E_INCOMPLETE_MESSAGE) // get the data
        {
            cbData = recv(Socket, (char*)pbIoBuffer + cbIoBuffer, cbIoBufferLength - cbIoBuffer, 0);
            if (cbData == SOCKET_ERROR)
            {
                printf("**** Error %d reading data from server\n", WSAGetLastError());
                scRet = SEC_E_INTERNAL_ERROR;
                break;
            }
            else if (cbData == 0) // Server disconnected.
            {
                if (cbIoBuffer)
                {
                    printf("**** Server unexpectedly disconnected\n");
                    scRet = SEC_E_INTERNAL_ERROR;
                    return scRet;
                }
                else
                    break; // All Done
            }
            else // success
            {
                printf("%d bytes of (encrypted) application data received\n", cbData);
                if (fVerbose) { PrintHexDump(cbData, pbIoBuffer + cbIoBuffer); printf("\n"); }
                cbIoBuffer += cbData;
            }
        }


        // Decrypt the received data.
        Buffers[0].pvBuffer = pbIoBuffer;
        Buffers[0].cbBuffer = cbIoBuffer;
        Buffers[0].BufferType = SECBUFFER_DATA;  // Initial Type of the buffer 1
        Buffers[1].BufferType = SECBUFFER_EMPTY; // Initial Type of the buffer 2
        Buffers[2].BufferType = SECBUFFER_EMPTY; // Initial Type of the buffer 3
        Buffers[3].BufferType = SECBUFFER_EMPTY; // Initial Type of the buffer 4

        Message.ulVersion = SECBUFFER_VERSION;    // Version number
        Message.cBuffers = 4;                                    // Number of buffers - must contain four SecBuffer structures.
        Message.pBuffers = Buffers;                        // Pointer to array of buffers
        scRet = g_pSSPI->DecryptMessage(phContext, &Message, 0, NULL);
        if (scRet == SEC_I_CONTEXT_EXPIRED) break; // Server signalled end of session
        //      if( scRet == SEC_E_INCOMPLETE_MESSAGE - Input buffer has partial encrypted record, read more
        if (scRet != SEC_E_OK &&
            scRet != SEC_I_RENEGOTIATE &&
            scRet != SEC_I_CONTEXT_EXPIRED)
        {
            printf("**** DecryptMessage ");
            DisplaySECError((DWORD)scRet);
            return scRet;
        }



        // Locate data and (optional) extra buffers.
        pDataBuffer = NULL;
        pExtraBuffer = NULL;
        for (i = 1; i < 4; i++)
        {
            if (pDataBuffer == NULL && Buffers[i].BufferType == SECBUFFER_DATA) pDataBuffer = &Buffers[i];
            if (pExtraBuffer == NULL && Buffers[i].BufferType == SECBUFFER_EXTRA) pExtraBuffer = &Buffers[i];
        }


        // Display the decrypted data.
        if (pDataBuffer)
        {
            length = pDataBuffer->cbBuffer;
            if (length) // check if last two chars are CR LF
            {
                buff = (PBYTE)pDataBuffer->pvBuffer; // printf( "n-2= %d, n-1= %d \n", buff[length-2], buff[length-1] );
                printf("Decrypted data: %d bytes", length); PrintText(length, buff);
                if (fVerbose) { PrintHexDump(length, buff); printf("\n"); }
                if (buff[length - 2] == 13 && buff[length - 1] == 10) break; // printf("Found CRLF\n");
            }
        }



        // Move any "extra" data to the input buffer.
        if (pExtraBuffer)
        {
            MoveMemory(pbIoBuffer, pExtraBuffer->pvBuffer, pExtraBuffer->cbBuffer);
            cbIoBuffer = pExtraBuffer->cbBuffer; // printf("cbIoBuffer= %d  \n", cbIoBuffer);
        }
        else
            cbIoBuffer = 0;


        // The server wants to perform another handshake sequence.
        if (scRet == SEC_I_RENEGOTIATE)
        {
            printf("Server requested renegotiate!\n");
            scRet = ClientHandshakeLoop(Socket, phCreds, phContext, FALSE, &ExtraBuffer);
            if (scRet != SEC_E_OK) return scRet;

            if (ExtraBuffer.pvBuffer) // Move any "extra" data to the input buffer.
            {
                MoveMemory(pbIoBuffer, ExtraBuffer.pvBuffer, ExtraBuffer.cbBuffer);
                cbIoBuffer = ExtraBuffer.cbBuffer;
            }
        }
    } // Loop till CRLF is found at the end of the data

    return SEC_E_OK;
}



/*****************************************************************************/
static SECURITY_STATUS SMTPsession(SOCKET          Socket,     // in
    PCredHandle     phCreds,    // in
    CtxtHandle* phContext)  // in
{
    SecPkgContext_StreamSizes Sizes;            // unsigned long cbBuffer;    // Size of the buffer, in bytes
    SECURITY_STATUS                        scRet;            // unsigned long BufferType;  // Type of the buffer (below)        
    PBYTE                                            pbIoBuffer; // void    SEC_FAR * pvBuffer;   // Pointer to the buffer
    DWORD                                            cbIoBufferLength, cbData;


    // Read stream encryption properties.
    scRet = g_pSSPI->QueryContextAttributes(phContext, SECPKG_ATTR_STREAM_SIZES, &Sizes);
    if (scRet != SEC_E_OK)
    {
        printf("**** Error 0x%x reading SECPKG_ATTR_STREAM_SIZES\n", scRet); return scRet;
    }


    // Create a buffer.
    cbIoBufferLength = Sizes.cbHeader + Sizes.cbMaximumMessage + Sizes.cbTrailer;
    pbIoBuffer = (PBYTE) LocalAlloc(LMEM_FIXED, cbIoBufferLength);
    if (pbIoBuffer == NULL) { printf("**** Out of memory (2)\n"); return SEC_E_INTERNAL_ERROR; }


    // Receive a Response
    scRet = ReadDecrypt(Socket, phCreds, phContext, pbIoBuffer, cbIoBufferLength);
    if (scRet != SEC_E_OK) return scRet;


    // Build the request - must be < maximum message size
    sprintf((char*)pbIoBuffer + Sizes.cbHeader, "%s", "EHLO \r\n"); // message begins after the header


    // Send a request.
    cbData = EncryptSend(Socket, phContext, pbIoBuffer, Sizes);
    if (cbData == SOCKET_ERROR || cbData == 0)
    {
        printf("**** Error %d sending data to server (3)\n", WSAGetLastError()); return SEC_E_INTERNAL_ERROR;
    }


    // Receive a Response
    scRet = ReadDecrypt(Socket, phCreds, phContext, pbIoBuffer, cbIoBufferLength);
    if (scRet != SEC_E_OK) return scRet;




    // Build the request - must be < maximum message size
    sprintf((char*)pbIoBuffer + Sizes.cbHeader, "%s", "QUIT \r\n"); // message begins after the header


    // Send a request.
    cbData = EncryptSend(Socket, phContext, pbIoBuffer, Sizes);
    if (cbData == SOCKET_ERROR || cbData == 0)
    {
        printf("**** Error %d sending data to server (3)\n", WSAGetLastError()); return SEC_E_INTERNAL_ERROR;
    }


    // Receive a Response
    scRet = ReadDecrypt(Socket, phCreds, phContext, pbIoBuffer, cbIoBufferLength);
    if (scRet != SEC_E_OK) return scRet;


    return SEC_E_OK;
}


/*****************************************************************************/
int main(int argc, char* argv[])
{
    WSADATA WsaData;
    SOCKET  Socket = INVALID_SOCKET;

    CredHandle hClientCreds;
    CtxtHandle hContext;
    BOOL fCredsInitialized = FALSE;
    BOOL fContextInitialized = FALSE;

    SecBuffer  ExtraData;
    SECURITY_STATUS Status;

    PCCERT_CONTEXT pRemoteCertContext = NULL;



    if (!LoadSecurityLibrary())
    {
        printf("Error initializing the security library\n"); goto cleanup;
    } //
    printf("----- SSPI Initialized\n");


    // Initialize the WinSock subsystem.
    if (WSAStartup(0x0101, &WsaData) == SOCKET_ERROR) // Winsock.h
    {
        printf("Error %d returned by WSAStartup\n", GetLastError()); goto cleanup;
    } //
    printf("----- WinSock Initialized\n");


    // Create credentials.
    if (CreateCredentials(pszUser, &hClientCreds))
    {
        printf("Error creating credentials\n"); goto cleanup;
    }
    fCredsInitialized = TRUE; //
    printf("----- Credentials Initialized\n");


    // Connect to server.
    if (ConnectToServer(pszServerName, iPortNumber, &Socket))
    {
        printf("Error connecting to server\n"); goto cleanup;
    } //
    printf("----- Connectd To Server\n");



    // Perform handshake
    if (PerformClientHandshake(Socket, &hClientCreds, pszServerName, &hContext, &ExtraData))
    {
        printf("Error performing handshake\n"); goto cleanup;
    }
    fContextInitialized = TRUE; //
    printf("----- Client Handshake Performed\n");


    // Authenticate server's credentials. Get server's certificate.
    Status = g_pSSPI->QueryContextAttributes(&hContext, SECPKG_ATTR_REMOTE_CERT_CONTEXT, (PVOID)&pRemoteCertContext);
    if (Status != SEC_E_OK)
    {
        printf("Error 0x%x querying remote certificate\n", Status); goto cleanup;
    } //
    printf("----- Server Credentials Authenticated \n");


    // Display server certificate chain.
    DisplayCertChain(pRemoteCertContext, FALSE); //
    printf("----- Certificate Chain Displayed \n");


    // Attempt to validate server certificate.
    Status = VerifyServerCertificate(pRemoteCertContext, pszServerName, 0);
    if (Status) { printf("**** Error 0x%x authenticating server credentials!\n", Status); goto cleanup; }
    // The server certificate did not validate correctly. At this point, we cannot tell
    // if we are connecting to the correct server, or if we are connecting to a
    // "man in the middle" attack server - Best to just abort the connection.
    printf("----- Server Certificate Verified\n");



    // Free the server certificate context.
    CertFreeCertificateContext(pRemoteCertContext);
    pRemoteCertContext = NULL; //
    printf("----- Server certificate context released \n");


    // Display connection info.
    DisplayConnectionInfo(&hContext); //
    printf("----- Secure Connection Info\n");



    // Send Request, recover response. LPSTR pszRequest = "EHLO";
    if (SMTPsession(Socket, &hClientCreds, &hContext))
    {
        printf("Error SMTP Session \n"); goto cleanup;
    } //
    printf("----- SMTP session Complete \n");


    // Send a close_notify alert to the server and close down the connection.
    if (DisconnectFromServer(Socket, &hClientCreds, &hContext))
    {
        printf("Error disconnecting from server\n"); goto cleanup;
    }
    fContextInitialized = FALSE;
    Socket = INVALID_SOCKET; //
    printf("----- Disconnected From Server\n");




cleanup: //
    printf("----- Begin Cleanup\n");

    // Free the server certificate context.
    if (pRemoteCertContext)
    {
        CertFreeCertificateContext(pRemoteCertContext);
        pRemoteCertContext = NULL;
    }

    // Free SSPI context handle.
    if (fContextInitialized)
    {
        g_pSSPI->DeleteSecurityContext(&hContext);
        fContextInitialized = FALSE;
    }

    // Free SSPI credentials handle.
    if (fCredsInitialized)
    {
        g_pSSPI->FreeCredentialsHandle(&hClientCreds);
        fCredsInitialized = FALSE;
    }

    // Close socket.
    if (Socket != INVALID_SOCKET) closesocket(Socket);

    // Shutdown WinSock subsystem.
    WSACleanup();

    // Close "MY" certificate store.
    if (hMyCertStore) CertCloseStore(hMyCertStore, 0);

    UnloadSecurityLibrary();


    printf("----- All Done ----- \n");

    return 0;
}