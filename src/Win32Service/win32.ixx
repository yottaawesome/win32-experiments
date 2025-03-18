module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wtsapi32.h>
#include <userenv.h>
#include <Lmcons.h>

export module win32;

namespace Win32
{
	template<auto VValue>
	struct Constant
	{
		operator decltype(VValue)() const noexcept
		{
			return VValue;
		}
	};
}

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
		::PROCESS_INFORMATION,
		::STARTUPINFO,
		::HKEY,
		::LONG,
		::LPPROFILEINFOW,
		::PROFILEINFOW,
		::BOOL,
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
		::WaitForSingleObject,
		::WTSGetActiveConsoleSessionId,
		::CloseHandle,
		::WTSQueryUserToken,
		::DuplicateTokenEx,
		::CreateEnvironmentBlock,
		::DestroyEnvironmentBlock,
		::CreateProcessAsUserW,
		::RegGetValueW,
		::LoadUserProfileW,
		::GetCurrentProcessToken,
		::ImpersonateLoggedOnUser,
		::RevertToSelf,
		::GetUserNameW,
		::UnloadUserProfile
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
	constexpr auto CreateNewConsole = CREATE_NEW_CONSOLE;
	constexpr auto CreateBreakawayJob = CREATE_BREAKAWAY_FROM_JOB;
	constexpr auto CreateUnicodeEnvironment = CREATE_UNICODE_ENVIRONMENT;

	namespace Token
	{
		constexpr auto Read = TOKEN_READ;
		constexpr auto Duplicate = TOKEN_DUPLICATE;
		constexpr auto Query = TOKEN_QUERY;
		constexpr auto AssignPrimary = TOKEN_ASSIGN_PRIMARY;
		constexpr auto Impersonate = TOKEN_IMPERSONATE;
	}

	constexpr auto RrfRtRegSz = RRF_RT_REG_SZ;

	Constant<HKEY_CURRENT_USER> HKCU;
}
