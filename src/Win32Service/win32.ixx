module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module win32;

export namespace Win32
{
	using 
		::HANDLE,
		::DWORD,
		::LPWSTR,
		::SERVICE_TABLE_ENTRY,
		::LPSERVICE_MAIN_FUNCTIONW,
		::SC_HANDLE,
		::WCHAR,
		::SERVICE_STATUS,
		::SERVICE_STATUS_HANDLE,
		::WTSSESSION_NOTIFICATION,
		::LPVOID,
		::USHORT,
		::SetEvent,
		::CreateEventW,
		::GetModuleFileNameW,
		::RegisterEventSourceW,
		::StartServiceCtrlDispatcherW,
		::MultiByteToWideChar,
		::GetLastError,
		::WideCharToMultiByte,
		::FormatMessageA,
		::FormatMessageW,
		::LocalFree,
		::OpenSCManagerW,
		::OpenServiceW,
		::CreateServiceW,
		::CloseServiceHandle,
		::DeleteService,
		::SetServiceStatus,
		::RegisterServiceCtrlHandlerW,
		::RegisterServiceCtrlHandlerExW,
		::WaitForSingleObject
		;

	namespace FormatMessageFlags
	{
		enum
		{
			AllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER,
			FromSystem = FORMAT_MESSAGE_FROM_SYSTEM,
			IgnoreInserts = FORMAT_MESSAGE_IGNORE_INSERTS
		};
	}

	constexpr auto CpUtf8 = CP_UTF8;
	constexpr auto WcNoBestFitChars = WC_NO_BEST_FIT_CHARS;
	constexpr auto MaxPath = MAX_PATH;
	constexpr auto ScManagerAllAccess = SC_MANAGER_ALL_ACCESS;
	constexpr auto ServiceAllAccess = SERVICE_ALL_ACCESS;
	constexpr auto ServiceWin32OwnProcess = SERVICE_WIN32_OWN_PROCESS;
	constexpr auto ServiceDemandStart = SERVICE_DEMAND_START;
	constexpr auto ServiceErrorNormal = SERVICE_ERROR_NORMAL;

	constexpr auto Delete = DELETE;

	// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_status_process
	constexpr auto ServiceAcceptStop = SERVICE_ACCEPT_STOP;
	constexpr auto ServiceAccepSessionChange = SERVICE_ACCEPT_SESSIONCHANGE;
	constexpr auto ServiceStartPending = SERVICE_START_PENDING;
	constexpr auto ServiceControlShutdown = SERVICE_CONTROL_SHUTDOWN;
	constexpr auto ServiceRunning = SERVICE_RUNNING;
	constexpr auto ServiceStopped = SERVICE_STOPPED;
	constexpr auto ServiceStopPending = SERVICE_STOP_PENDING;
	constexpr auto NoError = NO_ERROR;

	constexpr auto ServiceControlStop = SERVICE_CONTROL_STOP;
	constexpr auto ServiceControlInterrogate = SERVICE_CONTROL_INTERROGATE;
	constexpr auto ServiceControlSessionChange = SERVICE_CONTROL_SESSIONCHANGE;

	constexpr auto Infinite = INFINITE;
}
