module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module win32;

export namespace Win32
{
	using
		::CreateNamedPipeW,
		::CloseHandle,
		::GetLastError,
		::FormatMessageA,
		::LocalFree,
		::ConnectNamedPipe,
		::CreateEventW,
		::CancelIo,
		::CreateFileA,
		::CreateFileW,
		::DisconnectNamedPipe,
		::WaitForSingleObject,
		::SetNamedPipeHandleState,
		::ReadFile,
		::WriteFile,
		::OutputDebugStringA,
		::OutputDebugStringW,
		::CreateProcess,
		::DWORD,
		::OVERLAPPED,
		::HANDLE,
		::BOOL,
		::LPVOID,
		::HMODULE,
		::SECURITY_ATTRIBUTES,
		::PROCESS_INFORMATION,
		::STARTUPINFO
		;

	template<auto VValue>
	struct Constant
	{
		operator decltype(VValue)() const noexcept
		{
			return VValue;
		}

		bool operator==(decltype(VValue) other) const noexcept
		{
			return VValue == other;
		}
	};
	constexpr Constant<INVALID_HANDLE_VALUE> InvalidHandleValue;

	namespace Pipes
	{
		// dwOpenMode
		namespace OpenMode
		{
			namespace Mode
			{
				enum
				{
					Duplex = PIPE_ACCESS_DUPLEX,
					Inbound = PIPE_ACCESS_INBOUND,
					Outbound = PIPE_ACCESS_OUTBOUND,
				};
			}
			namespace Flags
			{
				enum
				{
					FirstPipeInstance = FILE_FLAG_FIRST_PIPE_INSTANCE,
					WriteThrough = FILE_FLAG_WRITE_THROUGH,
					Overlapped = FILE_FLAG_OVERLAPPED,
				};
			}
		}
		
		// dwPipeMode
		namespace PipeMode
		{
			namespace Type
			{
				enum
				{
					Byte = PIPE_TYPE_BYTE,
					Message = PIPE_TYPE_MESSAGE
				};
			}
			namespace Read
			{
				enum
				{
					Byte = PIPE_READMODE_BYTE,
					Message = PIPE_READMODE_MESSAGE
				};
			}
			namespace Wait
			{
				enum
				{
					Wait = PIPE_WAIT,
					NoWait = PIPE_NOWAIT
				};
			}
			namespace RemoteClient
			{
				enum
				{
					AcceptRemoteClients = PIPE_ACCEPT_REMOTE_CLIENTS,
					RejectRemoteClients = PIPE_REJECT_REMOTE_CLIENTS
				};
			}
		}

		constexpr auto UnlimitedInstances = PIPE_UNLIMITED_INSTANCES;
	}

	namespace FormatMessageFlags
	{
		constexpr auto AllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER;
		constexpr auto FromSystem = FORMAT_MESSAGE_FROM_SYSTEM;
		constexpr auto IgnoreInserts = FORMAT_MESSAGE_IGNORE_INSERTS;
		constexpr auto FromHModule = FORMAT_MESSAGE_FROM_HMODULE;
	}

	namespace ErrorCodes
	{
		enum
		{
			IoPending = ERROR_IO_PENDING,
			PipeListening = ERROR_PIPE_LISTENING,
			MoreData = ERROR_MORE_DATA
		};
	}

	namespace AccessRights
	{
		enum
		{
			GenericRead = GENERIC_READ,
			GenericWrite = GENERIC_WRITE
		};
	}

	constexpr auto WaitTimeout = WAIT_TIMEOUT;
	constexpr auto WaitObject0 = WAIT_OBJECT_0;
	constexpr auto Infinite = INFINITE;
	constexpr auto OpenExisting = OPEN_EXISTING;

	constexpr auto DllProcessAttach = DLL_PROCESS_ATTACH;
	constexpr auto DllThreadAttach = DLL_THREAD_ATTACH;
	constexpr auto DllThreadDetach = DLL_THREAD_DETACH;
	constexpr auto DllProcessDetach = DLL_PROCESS_DETACH;
}