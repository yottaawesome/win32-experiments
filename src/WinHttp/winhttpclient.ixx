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
			return {};
		}

		void Post(const std::wstring& path, const std::string& body)
		{
			std::vector<const wchar_t*> accept;
			accept.push_back(L"application/json");
			DoRequest(L"POST", false, 8080, path, accept, std::wstring{ AdditionalHeaders }, body);
		}

	private:
		void DoRequest(
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

			WinHttpUniquePtr hConnect = WinHttpUniquePtr(
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
			WinHttpUniquePtr hRequest = WinHttpUniquePtr(
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

			std::string response = "";
			DWORD dwSize = 0;
			do
			{
				// Check for available data.
				if (not WinHttpQueryDataAvailable(hRequest.get(), &dwSize))
					throw HttpError(GetLastError(), "WinHttpQueryDataAvailable() failed");
				if (dwSize == 0)
					continue;

				// Allocate space for the buffer.
				std::string pszOutBuffer(dwSize, '\0');

				DWORD dwDownloaded = 0;
				bool succeeded = WinHttpReadData(
					hRequest.get(),
					pszOutBuffer.data(),
					dwSize,
					&dwDownloaded
				);
				if (not succeeded)
					throw HttpError(GetLastError(), "WinHttpReadData() failed");
				std::println("{}", pszOutBuffer);
				response += pszOutBuffer;
			} while (dwSize > 0);

			std::println("response:\n{}", response);
		}

		std::wstring m_server;
		WinHttpUniquePtr m_hSession;
	};
}