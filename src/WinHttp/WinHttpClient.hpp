#pragma once
#include <stdio.h>
#include <string>
#include <Windows.h>
#include <winhttp.h>

class WinHttpClient final
{
	public:
		WinHttpClient(std::wstring server);
		void PostJson(const std::wstring& path, const std::string& body);

	protected:
		const std::wstring m_server;
		HINTERNET m_hSession;
		void SendToApi(const std::wstring& serverPath, const std::string& jsonstr);
};

