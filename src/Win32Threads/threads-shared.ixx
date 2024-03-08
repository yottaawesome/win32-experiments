export module threads:shared;
import std;
import win32;

export
{
    struct HandleDeleter { void operator()(win32::HANDLE h) { win32::CloseHandle(h); } };
    using HandleUniquePtr = std::unique_ptr<std::remove_pointer_t<win32::HANDLE>, HandleDeleter>;

    struct system_category_error : public std::system_error
    {
        system_category_error(std::string_view msg, win32::DWORD errorCode = win32::GetLastError())
            : system_error(std::error_code{ static_cast<int>(errorCode), std::system_category() }, std::string{ msg })
        {}
    };

    template<size_t ThreadCount>
    struct SynchronizationBarrier final
    {
        public:
            ~SynchronizationBarrier()
            {
                win32::DeleteSynchronizationBarrier(&m_barrier);
            }

            SynchronizationBarrier()
            {
                // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializesynchronizationbarrier
                if (not win32::InitializeSynchronizationBarrier(&m_barrier, ThreadCount, -1))
                    throw system_category_error("InitializeSynchronizationBarrier() failed");
            }

        public:
            void Enter()
            {
                // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-entersynchronizationbarrier
                win32::EnterSynchronizationBarrier(&m_barrier, 0);
            }

        private:
            win32::SYNCHRONIZATION_BARRIER m_barrier;
    };
}
