export module misc;
import std;
import win32;

export namespace Transport
{
    struct CriticalSection final
    {
        CriticalSection(const CriticalSection&) = delete;
        CriticalSection& operator=(const CriticalSection&) = delete;

        CriticalSection()       { Win32::InitializeCriticalSection(&CS); }
        ~CriticalSection()      { Win32::DeleteCriticalSection(&CS); }
        void lock() noexcept    { Win32::EnterCriticalSection(&CS); }
        void unlock() noexcept  { Win32::LeaveCriticalSection(&CS); }

        Win32::CRITICAL_SECTION CS{};
    };
    using CriticalSectionScopedLock = std::scoped_lock<CriticalSection>;

    struct ListEntry
    {
        constexpr void Attach(const std::vector<std::byte>& data)
        {
            Data = data;
        }

        constexpr void Attach(std::span<std::byte> data)
        {
            Data = { data.begin(), data.end() };
        }

        constexpr size_t CopyTo(std::span<std::byte> out)
        {
            if (Index >= Data.size())
                return 0;
            size_t numberToCopy = std::min(Data.size() - Index, out.size());
            std::copy_n(Data.begin() + Index, numberToCopy, out.begin());
            Index += numberToCopy;
            return numberToCopy;
        }

        constexpr size_t CopyTo(std::vector<std::byte>& data)
        {
            if (Index >= Data.size())
                return 0;
            data.insert(data.end(), Data.begin() + Index, Data.end());
            Index += std::distance(Data.begin() + Index, Data.end());
            return Index;
        }

        constexpr bool Empty() const noexcept
        {
            return Index >= Data.size();
        }

        constexpr size_t Remaining() const noexcept
        {
            return Data.size() - Index;
        }

        ListEntry* Forward = nullptr;
        ListEntry* Backward = nullptr;
        std::vector<std::byte> Data;
        size_t Index = 0;
    };
    static_assert(
        []() consteval
        {
            ListEntry entry;
            if (entry.Remaining() != 0)
                throw std::runtime_error("Remaining() must be empty!");

            std::vector<std::byte> dataToEnter{ 
                std::byte{0x1}, std::byte{0x1}, std::byte{0x1}, std::byte{0x1}, std::byte{0x1},
                std::byte{0x1}, std::byte{0x1}, std::byte{0x1}, std::byte{0x1}, std::byte{0x1}
            };
            entry.Attach(dataToEnter);
            if (entry.Remaining() != dataToEnter.size())
                throw std::runtime_error("Remaining() must be v.size()!");

            std::array<std::byte, 5> arr{};
            entry.CopyTo(std::span{ arr.data(), arr.size() });
            if (entry.Remaining() != 5)
                throw std::runtime_error("Remaining() is expected to be 5.");
            if (not std::all_of(arr.begin(), arr.end(), [](auto x) {return x == std::byte{ 0x1 }; }))
                throw std::runtime_error("arr[0] is expected to be 0x1.");

            entry.CopyTo(std::span{ arr.data(), arr.size() });
            if (not entry.Empty() or entry.Remaining() != 0)
                throw std::runtime_error("entry is expected to be empty.");

            return true;
        }());

    // Double-linked list.
    struct List
    {
        constexpr List()
        {
            this->m_head.Forward = &this->m_head;
            this->m_head.Backward = &this->m_head;
        }

        constexpr ListEntry* Peek()
        {
            return this->m_head.Forward;
        }

        constexpr void RemoveHead()
        {
            ListEntry* first = this->m_head.Forward;
            ListEntry* previous = first->Backward;
            previous->Forward = first->Forward;
            first->Forward->Backward = first->Backward;
            delete first;
        }

        constexpr void AppendTail(ListEntry* entry)
        {
            ListEntry* last = m_head.Backward;
            ListEntry* next = last->Forward;

            last->Forward = entry;
            next->Backward = entry;
            entry->Forward = next;
            entry->Backward = last;
        }

        constexpr bool IsEmpty()
        {
            return (m_head.Forward == &m_head and m_head.Forward == &m_head);
        }

    private:
        ListEntry m_head;
    };

    // Abstract transport that allows writing and reading data. The goal here was to simplify sample's code,
    // so it does not have to take into the account all complications associated with using sockets or pipes.
    // However, it should be straightforward to replace this abstract transport with a concrete one list sockets/pipes/etc.
    struct Transport
    {
        void WriteData(std::span<std::byte> data)
        {
            if (data.empty())
                return;

            // Copy application buffer to the transport buffer.
            std::unique_ptr<ListEntry> entry = std::make_unique<ListEntry>();
            entry->Attach(data);

            // Add the entry to the list.
            CriticalSectionScopedLock cs(m_lock);
            m_list.AppendTail(entry.release());
        }

        size_t ReadData(std::span<std::byte> buffer)
        {
            CriticalSectionScopedLock cs(m_lock);
            // Read as much data as possible from the transport.
            size_t totalRead = 0;
            while (totalRead < buffer.size() and not m_list.IsEmpty())
            {
                ListEntry* first = m_list.Peek();
                // Copy data from the transport buffer to the application buffer.
                totalRead += first->CopyTo(buffer);
                if (first->Empty())
                    m_list.RemoveHead();
            }
            return totalRead;
        }

    private:
        CriticalSection m_lock;
        List m_list;
    };
}
