import std;
import win32;

namespace WinDnsSync
{
    void Run()
    {

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
    WinDnsAsync::Run();
    return 0;
}
