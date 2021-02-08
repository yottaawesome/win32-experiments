// ExportImportCertificate.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <memory>
#include <Windows.h>
#include <cryptuiapi.h>
#include <wincrypt.h>
#pragma comment(lib, "Cryptui.lib")
#pragma comment(lib, "Crypt32.lib")

void GetErrorCodeString(const DWORD errorCode, std::wstring& stringToHoldMessage) noexcept
{
    stringToHoldMessage = L"";

    DWORD flags =
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS;
    HMODULE handle = nullptr;

    void* ptrMsgBuf = nullptr;
    FormatMessageW(
        flags,
        handle,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // TODO this is deprecated
        (LPWSTR)&ptrMsgBuf,
        0,
        nullptr);

    if (ptrMsgBuf != nullptr)
    {
        stringToHoldMessage = (LPWSTR)ptrMsgBuf;
        LocalFree(ptrMsgBuf);
    }
    else
    {
        stringToHoldMessage += L" failed to translate Win32 error code: " + std::to_wstring(errorCode);
    }
}

std::shared_ptr<void> MakeStorePtr(HCERTSTORE store)
{
	return {
		store,
		[](HCERTSTORE store) { CertCloseStore(store, 0); }
	};
}

std::shared_ptr<CERT_CONTEXT> MakeCertPtr(CERT_CONTEXT* store)
{
	return {
		store,
		[](CERT_CONTEXT* ctx) { CertFreeCertificateContext(ctx); }
	};
}

int main()
{
    auto accountPersonalCertStore = MakeStorePtr(CertOpenSystemStoreW(0, L"MY"));
	
	if (accountPersonalCertStore == nullptr)
		return GetLastError();
	auto accountRootCertStore = MakeStorePtr(CertOpenSystemStoreW(0, L"ROOT"));
	if (accountRootCertStore == nullptr)
		return GetLastError();

	std::string name = "localhost";
	CERT_NAME_BLOB blob{
		.cbData = (DWORD)name.size()*sizeof(char),
		.pbData = (BYTE*)&name[0]
	};

	auto certContext = MakeCertPtr(
		(CERT_CONTEXT*)CertFindCertificateInStore(
			accountPersonalCertStore.get(),
			0
			| PKCS_7_ASN_ENCODING
			| X509_ASN_ENCODING //Errors out on some certs if this is specified
			| CERT_FIND_HAS_PRIVATE_KEY,
			0,
			
			//CERT_FIND_SUBJECT_NAME,
			//CERT_FIND_ISSUER_NAME,
			CERT_FIND_SUBJECT_STR,
			//CERT_FIND_ISSUER_STR,
			
			(void*)L"0331d46883293028",
			//(void*)L"client.localhost",
			//&blob,
			//(void*)L"Windows Azure Tools",
			
			nullptr
		)
	);

	//certContext->pCertInfo->Subject;
	
	
	if (certContext == nullptr)
	{
		std::wcout << "Not found" << std::endl;
		return GetLastError();
	}

	HCERTSTORE extraStores[1] = { accountRootCertStore.get() };
	// https://docs.microsoft.com/en-us/windows/win32/api/cryptuiapi/ns-cryptuiapi-cryptui_wiz_export_info
	CRYPTUI_WIZ_EXPORT_INFO exportInfo{
		.dwSize = sizeof(CRYPTUI_WIZ_EXPORT_INFO),
		.pwszExportFileName = L"exported-cert.pfx",
		.dwSubjectChoice = CRYPTUI_WIZ_EXPORT_CERT_CONTEXT,
		.pCertContext = certContext.get(),
		.cStores = 1,
		.rghStores = extraStores
	};

	std::wstring password = L"testpassword";
	// https://docs.microsoft.com/en-us/windows/win32/api/cryptuiapi/ns-cryptuiapi-cryptui_wiz_export_certcontext_info
	CRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO certContextInfo{
		.dwSize = sizeof(CRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO),
		.dwExportFormat = 
			CRYPTUI_WIZ_EXPORT_FORMAT_PFX,
			//CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7,
			//CRYPTUI_WIZ_EXPORT_FORMAT_BASE64,
		.fExportChain=false,
		.fExportPrivateKeys=true,
		.pwszPassword=password.c_str(),
		.fStrongEncryption = true,
	};

	// As per docs, these are not defined, so you must define them yourself
	const DWORD CRYPTUI_WIZ_EXPORT_PRIVATE_KEY = 0x0100;
	const DWORD CRYPTUI_WIZ_EXPORT_NO_DELETE_PRIVATE_KEY = 0x0200;

    // https://docs.microsoft.com/en-us/windows/win32/api/cryptuiapi/nf-cryptuiapi-cryptuiwizexport
    BOOL succeeded = CryptUIWizExport(
		CRYPTUI_WIZ_NO_UI,
        nullptr,
		nullptr,
        &exportInfo,
        &certContextInfo
    );
	if (succeeded == false)
	{
		DWORD lastError = GetLastError();
		std::wstring errorString;
		GetErrorCodeString(lastError, errorString);
		std::wcout 
			<< lastError
			<< ": "
			<< errorString
			<< std::endl;
		return lastError;
	}

    return 0;
}
