#include <Windows.h>
#include <Shlwapi.h>
#include <winhttp.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "winhttp.lib")

import std;

namespace DoRandom
{
    void Run()
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(2000, 6000); // distribution in range [1, 6]
        std::println("{}", dist6(rng));
    }
}

namespace Canonicalize
{
    void Run()
    {
        DWORD size = 150;
        std::wstring buffer(size, '\0');
        HRESULT result = UrlCanonicalizeW(
            L"http://example.com/path with spaces/file name.txt?query=value with spaces&param=another value",
            buffer.data(),
            &size,
            URL_ESCAPE_UNSAFE
        );
        if (FAILED(result))
            throw std::runtime_error("Failed to canonicalize URL");

        buffer.resize(size);
        std::wcout << std::format(L"{}", buffer);
    }
}

namespace Crack
{
    struct HostName
    {
        void Init(this auto&& self)
        {
            self.lpszHostName = self.HostNameBuffer.data();
            self.dwHostNameLength = static_cast<DWORD>(self.HostNameBuffer.size());
        }
        std::array<wchar_t, 450> HostNameBuffer{};
    };
    struct ExtraInfo
    {
        void Init(this auto&& self)
        {
            self.lpszExtraInfo = self.ExtraInfoBuffer.data();
            self.dwExtraInfoLength = static_cast<DWORD>(self.ExtraInfoBuffer.size());
        }
        std::array<wchar_t, 450> ExtraInfoBuffer{};
    };

    template<typename...TComponents>
    struct Components : URL_COMPONENTS, TComponents...
    {
        Components() : URL_COMPONENTS{ .dwStructSize = sizeof(URL_COMPONENTS) }
        {
            (TComponents::Init(), ...);
        }
    };

    void Run()
    {
        std::wstring string = L"http://example.com/path with spaces/file name.txt?query=value with spaces&param=another value";
        std::wstring string2 = L"http://example.com/path with spaces/file name.txt?query=value%20with%20spaces&param=another%20value";
        Components<ExtraInfo, HostName> components;
        bool result = WinHttpCrackUrl(
            string.data(),
            static_cast<DWORD>(string2.size()),
            ICU_ESCAPE,
            &components
        );
        if (not result)
            throw std::runtime_error("WinHttpCrackUrl failed");
        std::wcout << std::format(L"{} {}", components.lpszHostName, components.lpszExtraInfo);
    }
}

export extern "C" auto main() -> int
try
{
    Crack::Run();
    return 0;
}
catch (const std::exception& ex)
{
    std::println("Failed {}", ex.what());
}
