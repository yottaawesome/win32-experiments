export module misc;
import std;
import win32;

export namespace Transport
{
    struct CriticalSection
    {
        CriticalSection(Win32::CRITICAL_SECTION& cs)
            : CS(cs)
        {
            Win32::EnterCriticalSection(&CS);
        }

        ~CriticalSection()
        {
            Win32::LeaveCriticalSection(&CS);
        }

        Win32::CRITICAL_SECTION& CS;
    };

    struct ListEntry
    {
        void Attach(const std::vector<Win32::BYTE>& data)
        {
            this->data = data;
        }

        void CopyTo(std::vector<Win32::BYTE>& data, Win32::ULONG* bytesCopied)
        {
            data.insert(data.end(), this->data.begin(), this->data.end());
            *bytesCopied = this->data.size();
        }

        ListEntry* forward = nullptr;
        ListEntry* backward = nullptr;
        std::vector<Win32::BYTE> data;
    };

    // Double-linked list.
    struct List
    {
        List()
        {
            this->head.forward = &this->head;
            this->head.backward = &this->head;
        }

        ListEntry* Peek()
        {
            return this->head.forward;
        }

        void RemoveHead()
        {
            ListEntry* first = this->head.forward;
            ListEntry* previous = first->backward;
            previous->forward = first->forward;
            first->forward->backward = first->backward;

            delete first;
        }

        void AppendTail(ListEntry* entry)
        {
            ListEntry* last = this->head.backward;
            ListEntry* next = last->forward;

            last->forward = entry;
            next->backward = entry;
            entry->forward = next;
            entry->backward = last;
        }

        bool IsEmpty()
        {
            return (this->head.forward == &this->head && this->head.backward == &this->head);
        }

    private:
        ListEntry head;
    };

    // Abstract transport that allows writing and reading data. The goal here was to simplify sample's code,
    // so it does not have to take into the account all complications associated with using sockets or pipes.
    // However, it should be straightforward to replace this abstract transport with a concrete one list sockets/pipes/etc.
    struct Transport
    {
        Transport()
        {
            Win32::InitializeCriticalSection(&this->lock);
        }

        ~Transport()
        {
            Win32::DeleteCriticalSection(&this->lock);
        }

        void WriteData(Win32::BYTE* data, Win32::ULONG dataLength)
        {
            if (not data)
                return;

            std::vector<Win32::BYTE> buffer{ data, data+dataLength };

            std::unique_ptr<ListEntry> entry = std::make_unique<ListEntry>();

            // Copy application buffer to the transport buffer.
            entry->Attach(buffer);

            // Add the entry to the list.
            CriticalSection cs(this->lock);
            list.AppendTail(entry.release());
        }

        void ReadData(std::vector<Win32::BYTE>& data)
        {
            CriticalSection cs(this->lock);
            // Read as much data as possible from the transport.
            while (not list.IsEmpty())
            {
                ListEntry* first = list.Peek();
                // Copy data from the transport buffer to the application buffer.
                Win32::ULONG copiedBytes = 0;
                first->CopyTo(data, &copiedBytes);
                list.RemoveHead();
            }
        }

    private:
        Win32::CRITICAL_SECTION lock;
        List list;
    };
}
