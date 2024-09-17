#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
// See https://weblogs.asp.net/kennykerr/parallel-programming-with-c-part-4-i-o-completion-ports
class CompletionPort
{
public:

    CompletionPort() 
    {
        Create(0);
    }


    ~CompletionPort()
    {
        CloseHandle(m_h);
    }

    HRESULT Create(DWORD threadCount)
    {
        m_h = ::CreateIoCompletionPort(
            INVALID_HANDLE_VALUE,
            0, // no existing port
            0, // ignored
            threadCount
        );

        if (0 == m_h)
        {
            return HRESULT_FROM_WIN32(::GetLastError());
        }

        return S_OK;
    }

    HRESULT AssociateFile(
        HANDLE file,
        ULONG_PTR completionKey
    )
    {
        if (0 == ::CreateIoCompletionPort(
            file,
            m_h,
            completionKey,
            0)) // ignored
        {
            return HRESULT_FROM_WIN32(::GetLastError());
        }

        return S_OK;
    }

    HRESULT QueuePacket(
        DWORD bytesCopied,
        ULONG_PTR completionKey,
        OVERLAPPED* overlapped
    )
    {

        if (!::PostQueuedCompletionStatus(m_h,
            bytesCopied,
            completionKey,
            overlapped))
        {
            return HRESULT_FROM_WIN32(::GetLastError());
        }

        return S_OK;
    }

    HRESULT DequeuePacket(
        DWORD milliseconds,
        DWORD& bytesCopied,
        ULONG_PTR& completionKey,
        OVERLAPPED*& overlapped
    )
    {

        if (!::GetQueuedCompletionStatus(m_h,
            &bytesCopied,
            &completionKey,
            &overlapped,
            milliseconds))
        {
            return HRESULT_FROM_WIN32(::GetLastError());
        }

        return S_OK;
    }

private:

    CompletionPort(CompletionPort&) = delete;
    CompletionPort& operator=(CompletionPort&) = delete;
    HANDLE m_h;
};

int main()
{
    CompletionPort port;

    HRESULT result = port.Create(1);

    if (FAILED(result))
    {
        return 1;
        // Failed to create completion port. The HRESULT provides the reason.
    }

    HANDLE file(::CreateFile(L"testfile.txt",
        FILE_READ_DATA,
        FILE_SHARE_READ,
        0, // default security
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        0)); // no template

    if (INVALID_HANDLE_VALUE == file)
    {
        return 1;
        // Call GetLastError for more information.
    }

    result = port.AssociateFile(file,
        123); // completion key

    if (FAILED(result))
    {
        return 1;
        // Failed to associate file. The HRESULT provides the reason.
    }

    OVERLAPPED overlapped = { 0 };
    BYTE buffer[256] = { 0 };

    if (!::ReadFile(file,
        buffer,
        256,
        0, // ignored
        &overlapped))
    {
        // Call GetLastError for more information.
    }

    DWORD bytesCopied = 0;
    ULONG_PTR completionKey = 0;
    OVERLAPPED* overlappedPointer = 0;

    result = port.DequeuePacket(INFINITE,
        bytesCopied,
        completionKey,
        overlappedPointer);

    if (FAILED(result))
    {
        // Failed to dequeue a completion packet. The HRESULT provides the reason.
        return 1;
    }


    return 0;
}