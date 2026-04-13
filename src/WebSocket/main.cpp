#include <Windows.h>
#include <Websocket.h>
#include <winhttp.h>

#pragma comment(lib, "websocket.lib")
#pragma comment(lib, "winhttp.lib")

import std;

struct HInternetDeleter
{
	static auto operator()(HINTERNET handle) noexcept
	{
		WinHttpCloseHandle(handle);
	}
};
using HInternetUniquePtr = std::unique_ptr<std::remove_pointer_t<HINTERNET>, HInternetDeleter>;

auto GetLastErrorAsString() -> std::string
{
	// https://stackoverflow.com/questions/24145038/how-to-get-description-of-winhttp-errors-using-error-codes
	//Get the error message, if any.
	auto errorMessageID = DWORD{ GetLastError() };
	if (errorMessageID == 0)
		return {};

	auto handle = HMODULE{};
	auto flags = DWORD{ FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS };
	if (errorMessageID >= WINHTTP_ERROR_BASE and errorMessageID <= WINHTTP_ERROR_LAST)
	{
		GetModuleHandleEx(0, TEXT("winhttp.dll"), &handle);
		flags |= FORMAT_MESSAGE_FROM_HMODULE;
	}
	auto messageBuffer = static_cast<char*>(nullptr);
	auto size = DWORD{
		FormatMessageA(
			flags,
			handle,
			errorMessageID,
			0,
			reinterpret_cast<char*>(&messageBuffer),
			0,
			nullptr
		)};
	auto message = std::string(messageBuffer, size);
	while (message.ends_with('\0') or message.ends_with("\r") or message.ends_with("\n"))
		message.pop_back();
	LocalFree(messageBuffer);

	return message;
}

auto main() -> int
try
{
	auto server = std::wstring{ L"echo.websocket.org"};
	auto hSession = HINTERNET{
		WinHttpOpen(
			L"Example/1.0",
			WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0
		) };
	auto session = HInternetUniquePtr{ hSession };

	auto port = 443;// 51935;
	auto hConnectionHandle = HINTERNET{
		WinHttpConnect(
			hSession,
			server.c_str(),
			port,
			0
		)};
	if (not hConnectionHandle)
		throw std::runtime_error{ GetLastErrorAsString() };
	auto connectionHandle = HInternetUniquePtr{ hConnectionHandle };

	auto hRequestHandle = HINTERNET{
		WinHttpOpenRequest(
			hConnectionHandle,
			L"GET",
			L"/protocol",
			nullptr,
			nullptr,
			nullptr,
			WINHTTP_FLAG_SECURE
		)};
	if (not hRequestHandle)
		throw std::runtime_error{ GetLastErrorAsString() };
	auto requestHandle = HInternetUniquePtr{ hRequestHandle };

	auto dwFlags = DWORD{
		SECURITY_FLAG_IGNORE_UNKNOWN_CA |
		SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
		SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
		SECURITY_FLAG_IGNORE_CERT_DATE_INVALID
	};
	auto success = WinHttpSetOption(
		hRequestHandle,
		WINHTTP_OPTION_SECURITY_FLAGS,
		&dwFlags,
		sizeof(dwFlags)
	);
	if (not success)
		throw std::runtime_error{ GetLastErrorAsString() };

	success = WinHttpSetOption(
		hRequestHandle,
		WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
		nullptr,
		0
	);
	if (not success)
		throw std::runtime_error{ GetLastErrorAsString() };

	success = WinHttpSendRequest(
		hRequestHandle,
		WINHTTP_NO_ADDITIONAL_HEADERS,
		0,
		nullptr,
		0,
		0,
		0
	);
	if (not success)
		throw std::runtime_error{ GetLastErrorAsString() };

	if (not WinHttpReceiveResponse(hRequestHandle, 0))
		throw std::runtime_error{ GetLastErrorAsString() };

	auto hWebSocketHandle = HINTERNET{ WinHttpWebSocketCompleteUpgrade(hRequestHandle, 0) };
	if (not hWebSocketHandle)
		throw std::runtime_error{GetLastErrorAsString()};

	constexpr auto message = std::string_view{ "Hello world" };
	constexpr auto message2 = std::wstring_view{ L"Hello world" };
	auto dwError = WinHttpWebSocketSend(
		hWebSocketHandle,
		WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
		reinterpret_cast<void*>(const_cast<char*>(message.data())),
		DWORD{ message.size() * sizeof(char) }
	);
	if (dwError != ERROR_SUCCESS)
		throw std::runtime_error{ GetLastErrorAsString() };

	auto rgbBuffer = std::array<BYTE, 1024>{};
	auto dwBufferLength = static_cast<DWORD>(rgbBuffer.size());
	auto pbCurrentBufferPointer = rgbBuffer.data();
	auto dwBytesTransferred = DWORD{};
	auto charBuffer = std::string(1024, '\0');
	auto eBufferType = WINHTTP_WEB_SOCKET_BUFFER_TYPE{};
	do
	{
		if (dwBufferLength == 0)
			throw std::runtime_error{ GetLastErrorAsString() };
		dwError = WinHttpWebSocketReceive(
			hWebSocketHandle,
			&charBuffer[0],
			static_cast<DWORD>(charBuffer.size()),
			&dwBytesTransferred,
			&eBufferType
		);
		if (dwError != ERROR_SUCCESS)
			throw std::runtime_error{ GetLastErrorAsString() };
		// If we receive just part of the message restart the receive operation.
		pbCurrentBufferPointer += dwBytesTransferred;
		dwBufferLength -= dwBytesTransferred;
		//charBuffer[dwBytesTransferred] = '\0';
	} while (eBufferType == WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE);

	if (eBufferType != WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE)
		throw std::runtime_error{ GetLastErrorAsString() };

	charBuffer.shrink_to_fit();
	std::println("Received response from server: {}", charBuffer);

	dwError = WinHttpWebSocketClose(
		hWebSocketHandle,
		WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
		nullptr,
		0
	);

	auto status = USHORT{};
	auto closeReasonLength = DWORD{};
	auto closeReasonBuffer = std::array<BYTE, 123>{};
	dwError = WinHttpWebSocketQueryCloseStatus(
		hWebSocketHandle,
		&status,
		closeReasonBuffer.data(),
		static_cast<DWORD>(closeReasonBuffer.size()),
		&closeReasonLength
	);

    return 0;
}
catch (const std::exception& ex)
{
	std::println("{}", ex.what());
	return 1;
}
