#include <iostream>
#include <Windows.h>
#include <Websocket.h>
#include <winhttp.h>
#include <Assert.h>
#include <iostream>
#include <stdexcept>
#include <string>

#pragma comment(lib, "websocket.lib")
#pragma comment(lib, "winhttp.lib")

std::wstring GetLastErrorAsString()
{
	// https://stackoverflow.com/questions/24145038/how-to-get-description-of-winhttp-errors-using-error-codes
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::wstring(); //No error message has been recorded

	LPWSTR messageBuffer = nullptr;
	HMODULE handle = nullptr;
	DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS;
	if (errorMessageID >= WINHTTP_ERROR_BASE && errorMessageID <= WINHTTP_ERROR_LAST)
	{
		GetModuleHandleEx(0, TEXT("winhttp.dll"), &handle);
		flags |= FORMAT_MESSAGE_FROM_HMODULE;
	}
	size_t size = FormatMessageW(
		flags,
		handle,
		errorMessageID, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		(LPWSTR)&messageBuffer, 
		0,
		NULL);

	std::wstring message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

int main()
{
	std::wstring server(L"127.0.0.1");
	HINTERNET hSession = WinHttpOpen(
		L"Example/1.0",
		WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0
	);

	int port = 51935;
	HINTERNET hConnectionHandle = WinHttpConnect(
		hSession,
		server.c_str(),
		port,
		0);

	HINTERNET hRequestHandle = WinHttpOpenRequest(
		hConnectionHandle,
		L"GET",
		L"/protocol",
		NULL,
		NULL,
		NULL,
		WINHTTP_FLAG_SECURE);

	DWORD dwFlags =
		SECURITY_FLAG_IGNORE_UNKNOWN_CA |
		SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
		SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
		SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
	bool security = WinHttpSetOption(
		hRequestHandle,
		WINHTTP_OPTION_SECURITY_FLAGS,
		&dwFlags,
		sizeof(dwFlags)
	);
	if (!security)
	{
		throw std::runtime_error(std::to_string(GetLastError()));
	}

	bool websocket = WinHttpSetOption(
		hRequestHandle,
		WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
		NULL,
		0);
	if (!websocket)
	{
		throw std::runtime_error(std::to_string(GetLastError()));
	}

	BOOL fStatus = WinHttpSendRequest(
		hRequestHandle,
		WINHTTP_NO_ADDITIONAL_HEADERS,
		0,
		NULL,
		0,
		0,
		0);
	if (!fStatus)
	{
		std::wcout << GetLastErrorAsString() << std::endl;
		throw std::runtime_error(std::to_string(GetLastError()));
	}

	fStatus = WinHttpReceiveResponse(hRequestHandle, 0);
	if (!fStatus)
	{
		std::wcout << GetLastErrorAsString() << std::endl;
		throw std::runtime_error(std::to_string(GetLastError()));
	}

	HINTERNET hWebSocketHandle = WinHttpWebSocketCompleteUpgrade(hRequestHandle, NULL);
	if (hWebSocketHandle == NULL)
	{
		std::wcout << GetLastErrorAsString() << std::endl;
		throw std::runtime_error(std::to_string(GetLastError()));
	}
	WinHttpCloseHandle(hRequestHandle);
	hRequestHandle = NULL;

	const WCHAR* pcwszMessage = L"Hello world";
	const DWORD cdwMessageLength = ARRAYSIZE(L"Hello world") * sizeof(WCHAR);
	DWORD dwError = WinHttpWebSocketSend(hWebSocketHandle,
		WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
		(PVOID)pcwszMessage,
		cdwMessageLength);
	if (dwError != ERROR_SUCCESS)
	{
		throw std::runtime_error(std::to_string(GetLastError()));
	}

	BYTE rgbBuffer[1024];
	DWORD dwBufferLength = ARRAYSIZE(rgbBuffer);
	BYTE* pbCurrentBufferPointer = rgbBuffer;
	DWORD dwBytesTransferred = 0;

	std::string charBuffer;
	charBuffer.resize(1024);

	WINHTTP_WEB_SOCKET_BUFFER_TYPE eBufferType;
	do
	{
		if (dwBufferLength == 0)
		{
			throw std::runtime_error(std::to_string(GetLastError()));
		}

		dwError = WinHttpWebSocketReceive(
			hWebSocketHandle,
			&charBuffer[0],
			charBuffer.size(),
			&dwBytesTransferred,
			&eBufferType);
		if (dwError != ERROR_SUCCESS)
		{
			throw std::runtime_error(std::to_string(GetLastError()));
		}

		//
		// If we receive just part of the message restart the receive operation.
		//

		pbCurrentBufferPointer += dwBytesTransferred;
		dwBufferLength -= dwBytesTransferred;
		//charBuffer[dwBytesTransferred] = '\0';
	} while (eBufferType == WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE);

	if (eBufferType != WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE)
	{
		std::wcout << L"Unexpected buffer type\n" << std::endl;
		dwError = ERROR_INVALID_PARAMETER;
		throw std::runtime_error(std::to_string(GetLastError()));
	}

	charBuffer.shrink_to_fit();
	std::wcout << L"Received response from server: " << charBuffer.c_str() << std::endl;


	dwError = WinHttpWebSocketClose(hWebSocketHandle,
		WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
		NULL,
		0);

	USHORT usStatus = 0;
	DWORD dwCloseReasonLength = 0;
	BYTE rgbCloseReasonBuffer[123];
	dwError = WinHttpWebSocketQueryCloseStatus(
		hWebSocketHandle,
		&usStatus,
		rgbCloseReasonBuffer,
		ARRAYSIZE(rgbCloseReasonBuffer),
		&dwCloseReasonLength);

    return 0;
}

