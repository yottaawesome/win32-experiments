module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <websocket.h>

export module win32;

export namespace Win32
{
	using
		::HANDLE,
		::ULONG,
		::BYTE,
		::HRESULT,
		::CRITICAL_SECTION,
		::WEB_SOCKET_HTTP_HEADER,
		::WEB_SOCKET_HANDLE,
		::WEB_SOCKET_BUFFER,
		::WEB_SOCKET_BUFFER_TYPE,
		::WEB_SOCKET_ACTION,
		::WebSocketGetAction,
		::PVOID,
		::PCHAR,
		::WEB_SOCKET_ACTION_QUEUE,
		::WebSocketCreateClientHandle,
		::WebSocketCreateServerHandle,
		::WebSocketDeleteHandle,
		::WebSocketBeginClientHandshake,
		::WebSocketBeginServerHandshake,
		::WebSocketEndClientHandshake,
		::WebSocketEndServerHandshake,
		::WebSocketAbortHandle,
		::InitializeCriticalSection,
		::EnterCriticalSection,
		::LeaveCriticalSection,
		::DeleteCriticalSection
		;

	constexpr auto Failed(::HRESULT hr) noexcept
	{
		return FAILED(hr);
	}

	constexpr auto 
		S_Ok = S_OK,
		E_OutOfMemory = E_OUTOFMEMORY,
		E_Fail = E_FAIL;
}
