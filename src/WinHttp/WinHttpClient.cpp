#include "WinHttpClient.hpp"
#include <atlstr.h>
#include <stdexcept>

#pragma comment(lib, "winhttp.lib")

const static std::wstring AdditionalHeaders(L"Content-Type: application/json");

WinHttpClient::WinHttpClient(std::wstring server)
:	m_server(std::move(server)),
	m_hSession(nullptr)
{ }

void WinHttpClient::PostJson(const std::wstring& path, const std::string& body)
{
	SendToApi(path, body);
}

void WinHttpClient::SendToApi(const std::wstring& serverPath, const std::string& jsonstr)
{
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	BOOL  bResults = FALSE;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;
	CString response = "";

	// Use WinHttpOpen to obtain a session handle.
	m_hSession = WinHttpOpen(
		L"WinHttp Rest Client/1.0",
		WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0
	);

	hConnect = WinHttpConnect(
		m_hSession,
		m_server.c_str(),
		8080, // use any port
		//443, // for ssl
		0);

	// Create an HTTP request handle.
	if (!hConnect)
		throw std::runtime_error("Failed to connect");

	LPCWSTR acceptHeader[2] = { L"application/json", nullptr };

	// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopenrequest
	hRequest = WinHttpOpenRequest(
		hConnect,
		L"POST",
		serverPath.c_str(),
		NULL,
		WINHTTP_NO_REFERER,
		acceptHeader,
		0 // uncomment to use http
		// WINHTTP_FLAG_SECURE // uncomment to use ssl
	);

	if (hRequest) 
	{
		// Send a request.
		// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsendrequest
		bResults = WinHttpSendRequest(
			hRequest,
			AdditionalHeaders.c_str(),
			-1L,
			(LPVOID)jsonstr.c_str(),
			jsonstr.size(),
			jsonstr.size(),
			0
		);

		// End the request.
		if (bResults) 
		{
			bResults = WinHttpReceiveResponse(hRequest, NULL);

			// Keep checking for data until there is nothing left.
			if (bResults) 
			{
				do
				{
					// Check for available data.
					dwSize = 0;
					if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
						printf("Error %u in WinHttpQueryDataAvailable.\n", GetLastError());

					// Allocate space for the buffer.
					pszOutBuffer = new char[dwSize + 1];
					if (!pszOutBuffer)
					{
						printf("Out of memory\n");
						dwSize = 0;
					}
					else
					{
						// Read the Data.
						ZeroMemory(pszOutBuffer, dwSize + 1);

						if (!WinHttpReadData(
								hRequest,
								(LPVOID)pszOutBuffer,
								dwSize, &dwDownloaded)
						)
							printf("Error %u in WinHttpReadData.\n", GetLastError());
						else
							printf("%s\n", pszOutBuffer);
						response += pszOutBuffer;

						// Free the memory allocated to the buffer.
						delete[] pszOutBuffer;
					}

				} while (dwSize > 0);

				printf("response: \n%s\n", response);
			}
			// Report any errors.
			if (!bResults)
				printf("Error %d has occurred.\n", GetLastError());
		}
	}

	// Close any open handles.
	if (hRequest)
		WinHttpCloseHandle(hRequest);
	if (hConnect)
		WinHttpCloseHandle(hConnect);
	if (m_hSession)
		WinHttpCloseHandle(m_hSession);
}
