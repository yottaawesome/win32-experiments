#include "Wrapper.hpp"
#include <Windows.h>
#include <wtsapi32.h>
#include <iostream>
#include <stdexcept>
#include <sstream>

#pragma comment(lib, "wtsapi32.lib")

namespace WTS::Wrapper
{
	std::wstring GetTimeAsUtcString(const SYSTEMTIME& st)
	{
		// Format date buffer
		constexpr UINT dateStringLength = 9;
		wchar_t dateString[dateStringLength];
		GetDateFormatEx(
			LOCALE_NAME_INVARIANT,
			0,
			&st,
			L"yyyyMMdd",
			dateString,
			dateStringLength,
			nullptr
		);

		// Format time buffer
		constexpr UINT timeStringLength = 9;
		wchar_t timeString[timeStringLength];
		GetTimeFormatEx(
			LOCALE_NAME_INVARIANT,
			0,
			&st,
			L"HH:mm:ss",
			timeString,
			timeStringLength
		);

		TIME_ZONE_INFORMATION tzi;
		GetTimeZoneInformation(&tzi);
		std::wstringstream wss;
		wss << dateString
			<< L"-"
			<< timeString
			<< L"."
			<< st.wMilliseconds;
		DWORD actualBias = tzi.Bias * -1;
		if (actualBias >= 0)
			wss << L"+";
		wss << actualBias;
		return wss.str();
	}

	//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
	std::wstring GetLastErrorAsString()
	{
		//Get the error message, if any.
		DWORD errorMessageID = ::GetLastError();
		if (errorMessageID == 0)
			return std::wstring(); //No error message has been recorded

		LPWSTR messageBuffer = nullptr;
		size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

		std::wstring message(messageBuffer, size);

		//Free the buffer.
		LocalFree(messageBuffer);

		return message;
	}

	std::vector<Session> Querier::QuerySessions()
	{
		DWORD dwCount = 0;
		DWORD dwError;
		DWORD pLevel = 1;

		WTS_SESSION_INFO_1* sessions;
		if(WTSEnumerateSessionsEx(WTS_CURRENT_SERVER_HANDLE, &pLevel, 0, &sessions, &dwCount) == false)
			throw std::runtime_error("Could not enumerate WTS sessions");

		std::vector<Session> results;
		for (DWORD i = 0; i < dwCount; i++)
		{
			if (sessions[i].pUserName != nullptr)
			{
				std::wcout << sessions[i].SessionId << std::endl;
				std::wcout << sessions[i].pDomainName << std::endl;
				std::wcout << sessions[i].pUserName << std::endl;
				//std::wcout << sessions[i].pHostName << std::endl;
				std::wcout << sessions[i].pSessionName << std::endl;
				std::wcout << sessions[i].State << std::endl;

				wchar_t* buffer = nullptr;
				DWORD count = 0;
				WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, sessions[i].SessionId, WTSSessionInfo, &buffer, &count);
				WTSINFO* info = (WTSINFO*)buffer;
				std::wcout << info->LogonTime.QuadPart << std::endl;
				std::wcout << info->CurrentTime.QuadPart << std::endl;
				FILETIME ft;
				ft.dwLowDateTime = info->LogonTime.LowPart;
				ft.dwHighDateTime = info->LogonTime.HighPart;
				SYSTEMTIME st;
				FileTimeToSystemTime(&ft, &st);
				std::wcout << GetTimeAsUtcString(st) << std::endl;

				results.push_back(Session{
					std::wstring(sessions[i].pUserName),
					std::wstring(sessions[i].pDomainName),
					sessions[i].SessionId,
					sessions[i].State
				});
			}		
		}

		if(dwCount > 0 && WTSFreeMemoryEx(WTSTypeSessionInfoLevel1, sessions, dwCount) == false)
			throw std::runtime_error("Could not free WTS session memory");

		return results;
	}
}
