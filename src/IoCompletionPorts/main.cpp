import win32;

using namespace Win32;

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

    void Create(Win32::DWORD threadCount)
    {
        m_h = Win32::CreateIoCompletionPort(
            InvalidHandleValue,
            0, // no existing port
            0, // ignored
            threadCount
        );
        if (not m_h)
            throw Win32::Error::Win32Error();
    }

    void AssociateFile(Win32::HANDLE file, Win32::ULONG_PTR completionKey)
    {
        if (not Win32::CreateIoCompletionPort(
            file,
            m_h,
            completionKey,
            0)
        ) throw Win32::Error::Win32Error();
    }

    void QueuePacket(
        Win32::DWORD bytesCopied,
        Win32::ULONG_PTR completionKey,
        Win32::OVERLAPPED* overlapped
    )
    {
        if (not Win32::PostQueuedCompletionStatus(
            m_h,
            bytesCopied,
            completionKey,
            overlapped)
        ) throw Win32::Error::Win32Error();
    }

    void DequeuePacket(
        Win32::DWORD milliseconds,
        Win32::DWORD& bytesCopied,
        Win32::ULONG_PTR& completionKey,
        Win32::OVERLAPPED*& overlapped
    )
    {
        if (not Win32::GetQueuedCompletionStatus(
            m_h,
            &bytesCopied,
            &completionKey,
            &overlapped,
            milliseconds)
        ) throw Win32::Error::Win32Error();
    }

    private:
    CompletionPort(CompletionPort&) = delete;
    CompletionPort& operator=(CompletionPort&) = delete;
    Win32::HANDLE m_h;
};

int main() try
{
    CompletionPort port;
    port.Create(1);

    Win32::HANDLE file = Win32::CreateFileW(
        L"testfile.txt",
        Win32::FileReadData,
        Win32::FileShareRead,
        0, // default security
        Win32::OpenExisting,
        Win32::FileFlagOverlapped,
        0
    );
    if (Win32::InvalidHandleValue == file)
        throw Win32::Error::Win32Error();

    port.AssociateFile(file, 123); // completion key

    Win32::OVERLAPPED overlapped = { 0 };
    Win32::BYTE buffer[256] = { 0 };

    Win32::ReadFile(
        file,
        buffer,
        256,
        0, // ignored
        &overlapped
    );
    if (Win32::DWORD lastError = Win32::GetLastError(); 
        lastError != Win32::Error::Codes::IoPending
    ) throw Win32::Error::Win32Error(lastError);

    Win32::DWORD bytesCopied = 0;
    Win32::ULONG_PTR completionKey = 0;
    Win32::OVERLAPPED* overlappedPointer = 0;

    port.DequeuePacket(
        Win32::Infinite,
        bytesCopied,
        completionKey,
        overlappedPointer
    );

    return 0;
}
catch (const std::exception& ex)
{
    std::println("main() failed: {}", ex.what());
    return 1;
}