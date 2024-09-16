import std;
import win32;

namespace Converters
{
    std::string Convert(std::wstring_view wstr)
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
        {
            const auto lastError = Win32::GetLastError();
            throw std::runtime_error("WideCharToMultiByte() [1] failed");
        }

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
        {
            const auto lastError = Win32::GetLastError();
            throw std::runtime_error("WideCharToMultiByte() [2] failed");
        }

        return strTo;
    }
}

namespace std
{
    template<size_t N>
    struct formatter<std::array<wchar_t, N>, char> : formatter<char, char>
    {
        template <class TContext>
        auto format(const std::array<wchar_t, N>& str, TContext&& ctx) const
        {
            return format_to(ctx.out(), "{}", Converters::Convert(str.data()));
        }
    };
}

namespace
{
    template<typename...TArgs>
    void Print(std::format_string<TArgs...> msg, TArgs&&...args) 
    {
        std::println(msg, std::forward<TArgs>(args)...);
    }
}

namespace WinDnsSync
{
    
}

namespace GetAddrInfoNoCallback
{
    struct Context : Win32::OVERLAPPED
    {
        ~Context() 
        { 
            Win32::CloseHandle(ThreadHandle);
        }
        constexpr Context() = default;

        Win32::HANDLE Cancel = nullptr;
        Win32::ADDRINFOEXW* Result = nullptr;
        Win32::ADDRINFOEXW Hints{ .ai_family = Win32::IPAddressFamily::v4 };
        DWORD Status = 0;
        Win32::HANDLE ThreadHandle = Win32::OpenThread(Win32::ThreadSetContext, false, Win32::GetCurrentThreadId());
    };

    void Resolve(std::wstring_view name)
    {
        Win32::WSADATA wsaData{};
        int iResult = Win32::WSAStartup(Win32::MakeWord(2, 2), &wsaData);

        Context context{};

        int status = Win32::GetAddrInfoExW(
            name.data(),
            nullptr,
            Win32::ServiceNamespace::DNS,
            nullptr,
            &context.Hints,
            &context.Result,
            nullptr,
            &context,
            [](Win32::DWORD error, Win32::DWORD bytes, Win32::OVERLAPPED* op)
            {
                Context* c = reinterpret_cast<Context*>(op);
                c->Status = error;
                Win32::QueueUserAPC([](ULONG_PTR) {}, c->ThreadHandle, 0); // Not reliable
            },
            &context.Cancel
        );
        if (status == Win32::ErrorCodes::Success)
            std::println("Call completed synchronously.");
        else if (status == Win32::ErrorCodes::WsaIoPending)
            Win32::SleepEx(Win32::InfiniteWait, true);
        else
            throw std::runtime_error(std::format("GetAddrInfoExW() failed with {}.", status));

        for (Win32::ADDRINFOEXW* next = context.Result; next != nullptr; next = next->ai_next)
        {
            std::array<wchar_t, 64> buffer;
            Win32::DWORD outLength = static_cast<Win32::DWORD>(buffer.size());
            // https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsaaddresstostringw
            int status = Win32::WSAAddressToStringW(
                next->ai_addr,
                static_cast<DWORD>(next->ai_addrlen),
                nullptr,
                buffer.data(),
                &outLength
            );
            if (status == Win32::SocketError)
                Print("Failed with {}", status);
            else
                Print("Record: {}", buffer);
        }
    }

    void Run()
    {
        Resolve(L"www.google.com");
    }
}

namespace GetAddrInfo
{
    struct Context : Win32::OVERLAPPED
    {
        ~Context() { Win32::CloseHandle(hEvent); }
        Context() : 
            Win32::OVERLAPPED 
            { 
                .hEvent = 
                    []{
                        Win32::HANDLE hEvent = Win32::CreateEventW(nullptr, true, false, nullptr);
                        if (not hEvent)
                            throw std::runtime_error("Failed creatring hEvent.");
                        return hEvent; 
                    }()
            } 
        { }

        Win32::HANDLE Cancel = nullptr;
        Win32::ADDRINFOEXW* Result = nullptr;
        Win32::ADDRINFOEXW Hints{ .ai_family = Win32::IPAddressFamily::v4 };
        DWORD Status = 0;
    };

    void Resolve(std::wstring_view name)
    {
        Win32::WSADATA wsaData{};
        int iResult = Win32::WSAStartup(Win32::MakeWord(2, 2), &wsaData);

        Context context{};

        int status = Win32::GetAddrInfoExW(
            name.data(),
            nullptr,
            Win32::ServiceNamespace::DNS,
            nullptr,
            &context.Hints,
            &context.Result,
            nullptr,
            &context,
            nullptr, // Specifying a callback causes the call to fail when hEvent is specified.
            &context.Cancel
        );
        if (status == Win32::ErrorCodes::Success)
            std::println("Call completed synchronously.");
        else if (status == Win32::ErrorCodes::WsaIoPending)
            Win32::WaitForSingleObject(context.hEvent, Win32::InfiniteWait);
        else
            throw std::runtime_error(std::format("GetAddrInfoExW() failed with {}.", status));

        for (Win32::ADDRINFOEXW* next = context.Result; next != nullptr; next = next->ai_next)
        {
            std::array<wchar_t, 64> buffer;
            Win32::DWORD outLength = static_cast<Win32::DWORD>(buffer.size());
            // https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsaaddresstostringw
            int status = Win32::WSAAddressToStringW(
                next->ai_addr,
                static_cast<DWORD>(next->ai_addrlen),
                nullptr,
                buffer.data(),
                &outLength
            );
            if (status == Win32::SocketError)
                Print("Failed with {}", status);
            else
                Print("Record: {}", buffer);
        }
    }

    void Run()
    {
        Resolve(L"www.google.com");
    }
}

namespace WinDnsAsync
{
    struct Context 
    {
        ~Context() 
        { 
            Win32::CloseHandle(Completed); 
            if (Result.pQueryRecords)
                Win32::DnsFree(Result.pQueryRecords, Win32::DNS_FREE_TYPE::DnsFreeRecordList);
        }
        Win32::HANDLE Completed = 
            []{
                Win32::HANDLE handle = Win32::CreateEventW(nullptr, true, false, nullptr);
                if (not handle)
                    throw Util::Win32Error(Win32::GetLastError(), "Failed to create event.");
                return handle;
            }();
        Win32::DNS_QUERY_RESULT Result{ .Version = Win32::DnsQueryRequestVersion1 };
        Win32::DNS_QUERY_CANCEL CancelHandle{};
    };

    struct Record
    {
        std::wstring Address;
    };

    template<typename...TArgs>
    constexpr void Print(std::format_string<TArgs...> fmt, TArgs&&...args) noexcept
    {
        std::wcout << std::format(fmt, std::forward<TArgs>(args)...).data() << std::endl;
    }

    template<typename...TArgs>
    constexpr void Print(std::wformat_string<TArgs...> fmt, TArgs&&...args) noexcept
    {
        std::wcout << std::format(fmt, std::forward<TArgs>(args)...) << std::endl;
    }

    void Print() noexcept { std::wcout << L"\n"; }

    std::vector<Record> Resolve(std::wstring_view name)
    {
        Context context{};

        Win32::DNS_QUERY_REQUEST request{ 
            .Version = Win32::DnsQueryRequestVersion1,
            .QueryName = name.data(),
            .QueryType = Win32::DnsRecordType::AAAA,
            .QueryOptions = Win32::DnsOptions::DualAddress, // IPv4 mapped into IPv6
            .pDnsServerList = nullptr,
            .InterfaceIndex = 0,
            .pQueryCompletionCallback = // https://learn.microsoft.com/en-us/windows/win32/api/windns/nc-windns-dns_query_completion_routine
                [](void* context, Win32::DNS_QUERY_RESULT* result) 
                {
                    Win32::SetEvent(reinterpret_cast<Context*>(context)->Completed);
                }, 
            .pQueryContext = &context
        };

        switch (Win32::DNS_STATUS status = Win32::DnsQueryEx(&request, &context.Result, &context.CancelHandle))
        {
            case Win32::ErrorCodes::Success: /*Nothing to do here.*/
                break;
            case Win32::ErrorCodes::DnsRequestPending: 
                Win32::WaitForSingleObject(context.Completed, Win32::InfiniteWait);
                break;
            default:
                throw Util::Win32Error(status, "DnsQueryEx() failed");
        }

        if (context.Result.QueryStatus != Win32::ErrorCodes::Success)
            throw Util::Win32Error(context.Result.QueryStatus, "DnsQueryEx() failed");

        std::vector<Record> ips;
        for (Win32::DNS_RECORD* next = context.Result.pQueryRecords; next != nullptr; next = next->pNext)
        {
            std::array<wchar_t, 64> buffer;
            // This is fine, because IPv4 addresses can map into IPv6. See https://learn.microsoft.com/en-us/dotnet/api/system.net.ipaddress.isipv4mappedtoipv6?view=net-8.0.
            const wchar_t* status = Win32::InetNtopW(Win32::IPAddressFamily::v6, &next->Data.A.IpAddress, buffer.data(), buffer.size());
            if (not status)
            {
                Print(L"Failed converting IP address");
                continue;
            }
            ips.push_back(Record{ std::wstring{ buffer.data() } });
        }

        return ips;
    }

    void Run()
    try
    {
        [](auto&&...args)
        {
            ([](auto&& arg)
            {
                Print(L"Resolving IPs for {}:", arg);
                auto results = Resolve(arg);
                if (results.empty())
                    Print(L"   -> (No results)");
                for (auto&& ip : results)
                    Print(L"   -> {}", ip.Address);
                Print();

            }(args), ...);
        }(L"www.google.com", L"localhost", L"8.8.8.8");
    }
    catch (const std::exception& ex)
    {
        Print("Exception: {}", ex.what());
    }
}

int main()
{
    //WinDnsAsync::Run();
    //GetAddrInfo::Run();
    GetAddrInfoNoCallback::Run();
    return 0;
}
