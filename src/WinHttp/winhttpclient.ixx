module;

#include <Windows.h>
#include <winhttp.h>

export module winhttpclient;
import std;

namespace Client
{
	constexpr std::wstring_view AdditionalHeaders(L"Content-Type: application/json");

	template<auto VDeleteFn>
	struct Deleter
	{
		constexpr void operator()(HINTERNET h) { VDeleteFn(h); }
	};
	using WinHttpUniquePtr = std::unique_ptr<std::remove_pointer_t<HINTERNET>, Deleter<WinHttpCloseHandle>>;
}

export namespace Client
{
	struct HttpError : std::runtime_error
	{
		HttpError(std::uint32_t lastError, std::string_view msg)
			: std::runtime_error(std::format("{} -- {}", msg, lastError))
		{ }
	};

	struct HttpResponse
	{
		unsigned StatusCode = 0;
		std::string ResponseBody;
	};

	struct WinHttpClient final
	{
		WinHttpClient(std::wstring server)
			: m_server(std::move(server))
		{ }

		HttpResponse Get(const std::wstring& path)
		{
			return DoRequest(L"GET", true, 443, path, {}, L"", "");
		}

		void Post(const std::wstring& path, const std::string& body)
		{
			std::vector<const wchar_t*> accept;
			accept.push_back(L"application/json");
			DoRequest(L"POST", false, 8080, path, accept, std::wstring{ AdditionalHeaders }, body);
		}

	private:
		HttpResponse DoRequest(
			const std::wstring& method,
			bool secure,
			std::uint32_t port,
			const std::wstring& serverPath,
			const std::vector<const wchar_t*> setAcceptTypes,
			const std::wstring& additionalHeaders,
			const std::string& body
		)
		{

			// Use WinHttpOpen to obtain a session handle.
			m_hSession = WinHttpUniquePtr(
				WinHttpOpen(
					L"WinHttp Rest Client/1.0",
					WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
					WINHTTP_NO_PROXY_NAME,
					WINHTTP_NO_PROXY_BYPASS,
					0
				)
			);
			if (not m_hSession)
				throw HttpError(GetLastError(), "Failed to create handle");

			WinHttpUniquePtr hConnect(
				WinHttpConnect(
					m_hSession.get(),
					m_server.c_str(),
					port,
					0
				)
			);
			if (not hConnect)
				throw HttpError(GetLastError(), "Failed to connect");

			std::vector<const wchar_t*> acceptTypes = setAcceptTypes;
			acceptTypes.push_back(nullptr);

			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopenrequest
			WinHttpUniquePtr hRequest(
				WinHttpOpenRequest(
					hConnect.get(),
					method.c_str(),
					serverPath.c_str(),
					nullptr,
					WINHTTP_NO_REFERER,
					acceptTypes.data(),
					secure ? WINHTTP_FLAG_SECURE : 0
				)
			);
			if (not hRequest)
				throw HttpError(GetLastError(), "WinHttpOpenRequest() failed");

				// Send a request.
			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsendrequest
			bool bResults = WinHttpSendRequest(
				hRequest.get(),
				additionalHeaders.data(),
				-1L,
				reinterpret_cast<void*>(const_cast<char*>(body.c_str())),
				body.size(),
				body.size(),
				0
			);
			if (not bResults)
				throw HttpError(GetLastError(), "WinHttpSendRequest() failed");

			// Keep checking for data until there is nothing left.
			if (not WinHttpReceiveResponse(hRequest.get(), nullptr))
				throw HttpError(GetLastError(), "WinHttpReceiveResponse() failed");

			DWORD statusCode = 0;
			DWORD size = sizeof(statusCode);
			bResults = WinHttpQueryHeaders(
				hRequest.get(),
				WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
				WINHTTP_HEADER_NAME_BY_INDEX,
				&statusCode,
				&size, 
				WINHTTP_NO_HEADER_INDEX
			);
			if (not bResults)
				throw HttpError(GetLastError(), "WinHttpQueryHeaders() failed");

			std::string response;
			DWORD dwSize = 0;
			while (true)
			{
				// Check for available data.
				if (not WinHttpQueryDataAvailable(hRequest.get(), &dwSize))
					throw HttpError(GetLastError(), "WinHttpQueryDataAvailable() failed");
				if (dwSize == 0)
					break;

				// Allocate space for the buffer.
				size_t offset = response.size();
				response.resize(response.size() + dwSize);
				DWORD dwDownloaded = 0;
				bool succeeded = WinHttpReadData(
					hRequest.get(),
					response.data() + offset,
					dwSize,
					&dwDownloaded
				);
				if (not succeeded)
					throw HttpError(GetLastError(), "WinHttpReadData() failed");
			}

			return { statusCode, response };
		}

		std::wstring m_server;
		WinHttpUniquePtr m_hSession;
	};
}