// https://learn.microsoft.com/en-us/windows/win32/services/the-complete-service-sample
import std;
import win32;

namespace Utl
{
	auto TranslateErrorCode(Win32::DWORD errorCode) -> std::string
	{
		constexpr Win32::DWORD flags =
			Win32::FormatMessageFlags::AllocateBuffer
			| Win32::FormatMessageFlags::FromSystem
			| Win32::FormatMessageFlags::IgnoreInserts;

		void* messageBuffer = nullptr;
		Win32::FormatMessageA(
			flags,
			nullptr,
			errorCode,
			0,
			reinterpret_cast<char*>(&messageBuffer),
			0,
			nullptr
		);
		if (not messageBuffer)
		{
			const auto lastError = Win32::GetLastError();
			return std::format("FormatMessageA() failed on code {} with error {}", errorCode, lastError);
		}

		std::string msg(static_cast<char*>(messageBuffer));
		// This should never happen
		// See also https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-raisefailfastexception
		if (Win32::LocalFree(messageBuffer))
			std::abort();

		std::erase_if(msg, [](const char x) { return x == '\n' || x == '\r'; });
		return msg;
	}
}

namespace Error
{
	struct Win32Error : std::runtime_error
	{
		Win32Error(
			Win32::DWORD code,
			std::string_view msg,
			const std::source_location& loc = std::source_location::current(),
			const std::stacktrace& trace = std::stacktrace::current()
		) : runtime_error(Utl::TranslateErrorCode(code))
		{

		}
	};
}

namespace Utl
{
	std::string ConvertString(std::wstring_view wstr)
	{
		if (wstr.empty())
			return {};

		// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte
		// Returns the size in bytes, this differs from MultiByteToWideChar, which returns the size in characters
		const int sizeInBytes = Win32::WideCharToMultiByte(
			Win32::CpUtf8,										// CodePage
			Win32::WcNoBestFitChars,							// dwFlags 
			&wstr[0],										// lpWideCharStr
			static_cast<int>(wstr.size()),					// cchWideChar 
			nullptr,										// lpMultiByteStr
			0,												// cbMultiByte
			nullptr,										// lpDefaultChar
			nullptr											// lpUsedDefaultChar
		);
		if (sizeInBytes == 0)
			throw Error::Win32Error(Win32::GetLastError(), "WideCharToMultiByte() [1] failed");

		std::string strTo(sizeInBytes / sizeof(char), '\0');
		const int status = WideCharToMultiByte(
			Win32::CpUtf8,										// CodePage
			Win32::WcNoBestFitChars,							// dwFlags 
			&wstr[0],										// lpWideCharStr
			static_cast<int>(wstr.size()),					// cchWideChar 
			&strTo[0],										// lpMultiByteStr
			static_cast<int>(strTo.size() * sizeof(char)),	// cbMultiByte
			nullptr,										// lpDefaultChar
			nullptr											// lpUsedDefaultChar
		);
		if (status == 0)
			throw Error::Win32Error(Win32::GetLastError(), "WideCharToMultiByte() [2] failed");

		return strTo;
	}

	std::wstring ConvertString(std::string_view str)
	{
		if (str.empty())
			return {};

		// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
		// Returns the size in characters, this differs from WideCharToMultiByte, which returns the size in bytes
		int sizeInCharacters = Win32::MultiByteToWideChar(
			Win32::CpUtf8,									// CodePage
			0,											// dwFlags
			&str[0],									// lpMultiByteStr
			static_cast<int>(str.size() * sizeof(char)),// cbMultiByte
			nullptr,									// lpWideCharStr
			0											// cchWideChar
		);
		if (sizeInCharacters == 0)
			throw Error::Win32Error(Win32::GetLastError(), "MultiByteToWideChar() [1] failed");

		std::wstring wstrTo(sizeInCharacters, '\0');
		int status = Win32::MultiByteToWideChar(
			Win32::CpUtf8,									// CodePage
			0,											// dwFlags
			&str[0],									// lpMultiByteStr
			static_cast<int>(str.size() * sizeof(char)),	// cbMultiByte
			&wstrTo[0],									// lpWideCharStr
			static_cast<int>(wstrTo.size())				// cchWideChar
		);
		if (status == 0)
			throw Error::Win32Error(Win32::GetLastError(), "MultiByteToWideChar() [2] failed");

		return wstrTo;
	}
}

namespace RAII
{
	template<auto VDeleteFn>
	struct Deleter
	{
		void operator()(auto handle) const noexcept
		{
			VDeleteFn(handle);
		}
	};

	template<typename T, auto VDeleter>
	using UniquePtr = std::unique_ptr<T, Deleter<VDeleter>>;
	template<typename T, auto VDeleter>
	using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<T>, Deleter<VDeleter>>;

	using ServiceUniquePtr = IndirectUniquePtr<Win32::SC_HANDLE, Win32::CloseServiceHandle>;
}

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
				"{} {}",
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
					L"{} {}",
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

	void SvcInit(Win32::DWORD dwArgc, wchar_t* lpszArgv[])
	{
		// TO_DO: Declare and set any required variables.
		//   Be sure to periodically call ReportSvcStatus() with 
		//   SERVICE_START_PENDING. If initialization fails, call
		//   ReportSvcStatus with SERVICE_STOPPED.

		// Create an event. The control handler function, SvcCtrlHandler,
		// signals this event when it receives the stop control code.

		ghSvcStopEvent = Win32::CreateEventW(
			nullptr,    // default security attributes
			true,    // manual reset event
			false,   // not signaled
			nullptr);   // no name

		if (not ghSvcStopEvent)
		{
			ReportSvcStatus(Win32::ServiceStopped, Win32::GetLastError(), 0);
			return;
		}

		// Report running status when initialization is complete.

		ReportSvcStatus(Win32::ServiceRunning, Win32::NoError, 0);

		// TO_DO: Perform work until service stops.

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
			Log::Info("RegisterServiceCtrlHandler");
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
		std::wcerr << std::format("Service failed: {}\n", ex.what()).c_str();
	}
}

int __cdecl wmain(int argc, wchar_t* argv[])
try
{
    if (std::wstring str(argv[1]); str == L"install")
    {
        Service::Install();
        return 0;
    }
	else if (str == L"uninstall")
	{
		Service::Uninstall();
		return 0;
	}

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
	std::wcerr << std::format("Service failed: {}\n", ex.what()).c_str();
}
