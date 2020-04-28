#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <winhttp.h>

namespace WinHttp
{
	std::wstring Replace(std::wstring source, const std::wstring& from, const std::wstring& to);

	enum class WinHttpWebSocketStatus
	{
		NotInitialised,
		Connected,
		Closed,
		Error
	};

	class WinHttpHandle
	{
		public:
			WinHttpHandle();
			WinHttpHandle(HINTERNET handle);
			WinHttpHandle(const WinHttpHandle& copy) = delete;
			~WinHttpHandle();
			void operator=(const HINTERNET& copy);
			bool operator==(const HINTERNET other);
			HINTERNET Get();

		private:
			HINTERNET m_handle;
	};

	class WinHttpWebSocket final
	{
		public:
			WinHttpWebSocket(std::wstring server, const UINT port, const bool ignoreSslErrors);
			const std::wstring& GetServer();
			void Connect();
			void CleanServerString();
			void SendString(const std::string& msg);
			void SendBuffer(const std::vector<char>& buffer);
			bool Receive(std::string& buffer);
			void SetServer(const std::wstring& newServer, const UINT port, const bool ignoreSslErrors);
			void Close();
			WinHttpWebSocketStatus GetStatus();

		protected:
			WinHttpHandle m_hConnect;
			WinHttpHandle m_hSession;
			WinHttpHandle m_hWebSocketHandle;
			std::wstring m_server;
			UINT m_port;
			bool m_ignoreSslErrors;
			WinHttpWebSocketStatus m_status;
	};
}