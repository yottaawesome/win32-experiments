#pragma once
#include <vector>
#include <string>
#include <Windows.h>
#include <wtsapi32.h>

namespace WTS::Wrapper
{
	class Session
	{
		public:
			std::wstring UserName;
			std::wstring Domain;
			DWORD SessionId;
			WTS_CONNECTSTATE_CLASS State;
	};

	class Querier
	{
		public:
			std::vector<Session> QuerySessions();
	};
}