// https://learn.microsoft.com/en-us/windows/win32/services/the-complete-service-sample
import std;
import win32;
import common;

#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Userenv")

namespace Log
{
	void LogToFile(std::string_view message)
	{
		std::ofstream log;
		log.open("C:\\temp\\service-log.txt", std::ios::app);
		log << message.data();
		log.close();
	}

	template<typename...TArgs>
	void Info(std::format_string<TArgs...> fmt, TArgs&&...args)
	{
		auto const time = std::chrono::current_zone()
			->to_local(std::chrono::system_clock::now());

		LogToFile(
			std::format(
				"[{}] {}\n",
				std::format("{:%Y-%m-%d %X}", time),
				std::format(fmt, std::forward<TArgs>(args)...)));
	}

	template<typename...TArgs>
	void Info(std::wformat_string<TArgs...> fmt, TArgs&&...args)
	{
		auto const time = std::chrono::current_zone()
			->to_local(std::chrono::system_clock::now());

		LogToFile(
			Utl::ConvertString(
				std::format(
					L"[{}] {}\n",
					std::format(L"{:%Y-%m-%d %X}", time),
					std::format(fmt, std::forward<TArgs>(args)...))));
	}
}

namespace Service
{
    constexpr std::wstring_view ServiceName = L"ThisIsATestService";
	Win32::SERVICE_STATUS gSvcStatus{};
	Win32::SERVICE_STATUS_HANDLE gSvcStatusHandle = nullptr;
	Win32::HANDLE ghSvcStopEvent = nullptr;

    void Install()
    {
		wchar_t szPath[Win32::MaxPath];

		if (not Win32::GetModuleFileNameW(nullptr, szPath, Win32::MaxPath))
			throw Error::Win32Error(Win32::GetLastError(), "Cannot install service");

		auto scManager = RAII::ServiceUniquePtr(
			Win32::OpenSCManagerW(
				nullptr,
				nullptr,
				Win32::ScManagerAllAccess
			));
		if (not scManager)
			throw Error::Win32Error(Win32::GetLastError(), "Cannot OpenSCManager service");

		//Check if service exists
		auto service = RAII::ServiceUniquePtr(
			Win32::OpenServiceW(
				scManager.get(), 
				ServiceName.data(), 
				Win32::ServiceAllAccess
			));
		if (service)
			return;

		service = RAII::ServiceUniquePtr(
			Win32::CreateServiceW(
				scManager.get(),              // SCM database 
				ServiceName.data(),                   // name of service 
				ServiceName.data(),                   // service name to display 
				Win32::ServiceAllAccess,        // desired access 
				Win32::ServiceWin32OwnProcess, // service type 
				Win32::ServiceDemandStart,      // start type 
				Win32::ServiceErrorNormal,      // error control type 
				szPath,                    // path to service's binary 
				nullptr,                      // no load ordering group 
				nullptr,                      // no tag identifier 
				nullptr,                      // no dependencies 
				nullptr,                      // LocalSystem account 
				nullptr // no password 
			));                    
		if (not service)
			throw Error::Win32Error(Win32::GetLastError(), "Cannot OpenSCManager service");

		Log::Info("Service installed successfully");
    }

	void Uninstall()
	{
		auto schSCManager = RAII::ServiceUniquePtr(
			Win32::OpenSCManagerW(
				nullptr,
				nullptr,
				Win32::ScManagerAllAccess
			));
		if (not schSCManager)
			throw Error::Win32Error(Win32::GetLastError(), "OpenSCManager() failed");

		auto schService = RAII::ServiceUniquePtr(
			Win32::OpenServiceW(
				schSCManager.get(),
				ServiceName.data(),
				Win32::Delete
			));
		if (not schService)
			throw Error::Win32Error(Win32::GetLastError(), "OpenService() failed");

		if (not Win32::DeleteService(schService.get()))
			throw Error::Win32Error(Win32::GetLastError(), "DeleteService() failed");

		Log::Info("Service deleted successfully");
	}

	void ReportSvcStatus(
		Win32::DWORD dwCurrentState,
		Win32::DWORD dwWin32ExitCode,
		Win32::DWORD dwWaitHint
	)
	{
		static DWORD dwCheckPoint = 1;

		// Fill in the SERVICE_STATUS structure.

		gSvcStatus.dwCurrentState = dwCurrentState;
		gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
		gSvcStatus.dwWaitHint = dwWaitHint;

		if (dwCurrentState == Win32::ServiceStartPending)
			gSvcStatus.dwControlsAccepted = 0;
		else 
			gSvcStatus.dwControlsAccepted = Win32::ServiceAcceptStop | Win32::ServiceAccepSessionChange | Win32::ServiceControlShutdown;

		if (dwCurrentState == Win32::ServiceRunning or dwCurrentState == Win32::ServiceStopped)
			gSvcStatus.dwCheckPoint = 0;
		else 
			gSvcStatus.dwCheckPoint = dwCheckPoint++;

		// Report the status of the service to the SCM.
		Win32::SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
	}

	Win32::DWORD __stdcall SvcCtrlHandler(Win32::DWORD dwCtrl, Win32::DWORD eventType, Win32::LPVOID lpEventData, Win32::LPVOID lpContext)
	{
		switch (dwCtrl)
		{
			case Win32::ServiceControlInterrogate:
				return Win32::NoError;

			case Win32::ServiceControlStop:
				ReportSvcStatus(Win32::ServiceStopPending, Win32::NoError, 0);
				Win32::SetEvent(ghSvcStopEvent);
				ReportSvcStatus(gSvcStatus.dwCurrentState, Win32::NoError, 0);
				return Win32::NoError;

			case Win32::ServiceControlShutdown:
				Win32::SetEvent(ghSvcStopEvent);
				return Win32::NoError;

			case Win32::ServiceControlSessionChange:
			{
				Win32::WTSSESSION_NOTIFICATION* sessionData = (Win32::WTSSESSION_NOTIFICATION*)lpEventData;
				ReportSvcStatus(gSvcStatus.dwCurrentState, Win32::NoError, 0);
				return Win32::NoError;
			}

			default:
				break;
		}

		return 0;
	}

	void CreateUserProcess()
	try
	{
		// https://stackoverflow.com/questions/19796409/how-to-impersonate-a-user-from-a-service-correctly
		// https://stackoverflow.com/questions/26913172/create-process-in-user-session-from-service
		// https://web.archive.org/web/20101009012531/http://blogs.msdn.com/b/winsdk/archive/2009/07/14/launching-an-interactive-process-from-windows-service-in-windows-vista-and-later.aspx
		Win32::DWORD dwSessionID = Win32::WTSGetActiveConsoleSessionId();
		if (dwSessionID == 0xFFFFFFFF)
			throw Error::Win32Error(Win32::GetLastError(), "WTSGetActiveConsoleSessionId failed");

		Win32::HANDLE hToken = nullptr;
		if (not Win32::WTSQueryUserToken(dwSessionID, &hToken))
			throw Error::Win32Error(Win32::GetLastError(), "WTSQueryUserToken failed.");

		RAII::HandleUniquePtr userPrimaryToken(hToken);
		Win32::HANDLE hDuplicated = nullptr;
		bool success = Win32::DuplicateTokenEx(
			userPrimaryToken.get(),
			Win32::Token::Read | Win32::Token::AssignPrimary | Win32::Token::Duplicate | Win32::Token::Query,
			nullptr,
			SecurityIdentification,
			TokenPrimary,
			&hDuplicated
		);
		if (not success)
			throw Error::Win32Error(Win32::GetLastError(), "DuplicateToken failed.");
		RAII::HandleUniquePtr duplicatedToken(hDuplicated);

		void* environment = nullptr;
		if (not Win32::CreateEnvironmentBlock(&environment, duplicatedToken.get(), false))
			throw Error::Win32Error(Win32::GetLastError(), "CreateEnvironmentBlock failed.");

		RAII::EnvironmentUniquePtr environmentBlock(environment);

		Win32::PROCESS_INFORMATION info{ 0 };
		Win32::STARTUPINFO startup{ .cb = sizeof(startup) };
		std::wstring commandLine = L"command line";
		success = Win32::CreateProcessAsUserW(
			duplicatedToken.get(),
			LR"(A:\Code\cpp\win32-experiments\src\x64\Debug\UserProcess.exe)",
			commandLine.data(),
			nullptr,
			nullptr,
			false,
			Win32::CreateNewConsole | Win32::CreateUnicodeEnvironment | Win32::CreateBreakawayJob,
			environmentBlock.get(),
			L"C:\\temp",
			&startup,
			&info
		);
		if (not success)
			throw Error::Win32Error(Win32::GetLastError(), "CreateProcessAsUserW failed.");

		RAII::HandleUniquePtr hProc(info.hProcess);
		RAII::HandleUniquePtr hProcThread(info.hThread);
	}
	catch (const std::exception& ex)
	{
		Log::Info("Failed creating user process: {}", ex.what());
	}

	void ImpersonateUser()
	try
	{
		// TODO
		// GetProfileType();
		// Need to extract token and load their profile, since
		// impersonating does not load it.

		Win32::DWORD dwSessionID = Win32::WTSGetActiveConsoleSessionId();
		if (dwSessionID == 0xFFFFFFFF)
			throw Error::Win32Error(Win32::GetLastError(), "WTSGetActiveConsoleSessionId failed");

		Win32::HANDLE hToken = nullptr;
		if (not Win32::WTSQueryUserToken(dwSessionID, &hToken))
			throw Error::Win32Error(Win32::GetLastError(), "WTSQueryUserToken failed.");

		RAII::HandleUniquePtr userPrimaryToken(hToken);
		Win32::HANDLE hDuplicated = nullptr;
		bool success = Win32::DuplicateTokenEx(
			userPrimaryToken.get(),
			Win32::Token::Impersonate | Win32::Token::Query,
			nullptr,
			SecurityImpersonation,
			TokenImpersonation,
			&hDuplicated
		);
		if (not success)
			throw Error::Win32Error(Win32::GetLastError(), "DuplicateToken failed.");
		RAII::HandleUniquePtr duplicatedToken(hDuplicated);

		if (not Win32::ImpersonateLoggedOnUser(duplicatedToken.get()))
			throw Error::Win32Error(Win32::GetLastError(), "ImpersonateLoggedOnUser failed.");

		std::wstring username(256 + 1, '\0');
		Win32::DWORD size = static_cast<Win32::DWORD>(username.size());
		if (not Win32::GetUserNameW(username.data(), &size))
			throw Error::Win32Error(Win32::GetLastError(), "GetUserNameW failed");
		username.resize(size - 1);

		Win32::PROFILEINFOW profile{ .dwSize = sizeof(Win32::PROFILEINFOW), .lpUserName = username.data() };
		// Fails probably because "The calling process must have the SE_RESTORE_NAME and SE_BACKUP_NAME privileges.
		// See https://learn.microsoft.com/en-us/windows/win32/secauthz/enabling-and-disabling-privileges-in-c--
		if (not Win32::LoadUserProfileW(duplicatedToken.get(), &profile))
			throw Error::Win32Error(Win32::GetLastError(), "Failed to load profile");

		Win32::LoadUserProfileW(duplicatedToken.get(), &profile);

		Win32::RevertToSelf();
		Log::Info("Successful impersonation.");
	}
	catch (const std::exception& ex)
	{
		Win32::RevertToSelf();
		Log::Info("ImpersonateUser: {}\n", ex.what());
	}

	void SvcInit(Win32::DWORD dwArgc, wchar_t* lpszArgv[])
	{
		Log::Info("SvcInit()");
		//   Be sure to periodically call ReportSvcStatus() with 
		//   SERVICE_START_PENDING. If initialization fails, call
		//   ReportSvcStatus with SERVICE_STOPPED.

		// Create an event. The control handler function, SvcCtrlHandler,
		// signals this event when it receives the stop control code.
		ghSvcStopEvent = Win32::CreateEventW(
			nullptr,    // default security attributes
			true,    // manual reset event
			false,   // not signaled
			nullptr // no name
		);
		if (not ghSvcStopEvent)
		{
			ReportSvcStatus(Win32::ServiceStopped, Win32::GetLastError(), 0);
			return;
		}

		// Report running status when initialization is complete.
		ReportSvcStatus(Win32::ServiceRunning, Win32::NoError, 0);

		//CreateUserProcess();
		ImpersonateUser();
		
		while (true)
		{
			// Check whether to stop the service.
			Win32::WaitForSingleObject(ghSvcStopEvent, Win32::Infinite);

			ReportSvcStatus(Win32::ServiceStopped, Win32::NoError, 0);
			return;
		}
	}

    void __stdcall SvcMain(Win32::DWORD dwArgc, Win32::LPWSTR lpszArgv[])
	try
    {
		gSvcStatusHandle = Win32::RegisterServiceCtrlHandlerExW(
			ServiceName.data(),
			SvcCtrlHandler,
			0
		);
		if (!gSvcStatusHandle)
		{
			Log::Info("RegisterServiceCtrlHandler() failed.");
			return;
		}

		// These SERVICE_STATUS members remain as set here
		gSvcStatus.dwServiceType = Win32::ServiceWin32OwnProcess;
		gSvcStatus.dwServiceSpecificExitCode = 0;

		// Report initial status to the SCM

		ReportSvcStatus(Win32::ServiceStartPending, 0, 3000);

		// Perform service-specific initialization and work.

		SvcInit(dwArgc, lpszArgv);
    }
	catch (const std::exception& ex)
	{
		Log::Info("Service failed: {}\n", ex.what());
		std::wcerr << std::format("Service failed: {}\n", ex.what()).c_str();
	}
}

int __cdecl wmain(int argc, wchar_t* argv[])
try
{
	if (argc == 2)
	{
		std::wstring str(argv[1]);
		if (str == L"install")
		{
			Service::Install();
			return 0;
		}
		else if (str == L"uninstall")
		{
			Service::Uninstall();
			return 0;
		}
	}
    
	Log::Info("Running...");
    Win32::SERVICE_TABLE_ENTRY DispatchTable[] =
    {
        { const_cast<Win32::LPWSTR>(Service::ServiceName.data()), static_cast<Win32::LPSERVICE_MAIN_FUNCTIONW>(Service::SvcMain)},
        { nullptr, nullptr }
    };

    if (not Win32::StartServiceCtrlDispatcherW(DispatchTable))
        Log::Info(L"StartServiceCtrlDispatcher");

    return 0;
}
catch (const std::exception& ex)
{
	Log::Info("Service failed : {}\n", ex.what());
	std::wcerr << std::format("Service failed: {}\n", ex.what()).c_str();
	return 1;
}
