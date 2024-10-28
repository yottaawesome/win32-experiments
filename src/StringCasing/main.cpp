#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

import std;

// https://learn.microsoft.com/en-us/windows/win32/api/winnls/nf-winnls-lcmapstringex
// See also https://devblogs.microsoft.com/oldnewthing/20241007-00/?p=110345
std::wstring ChangeCase(const std::wstring& source, const bool upper)
{
    const int flag = upper ? LCMAP_UPPERCASE : LCMAP_LOWERCASE;

    std::wstring destination;
    int result = LCMapStringEx(
        LOCALE_NAME_INVARIANT,
        flag,
        source.data(),
        static_cast<int>(source.size()),
        destination.data(),
        0,
        nullptr,
        nullptr,
        0
    );
    if (result == 0)
        throw std::runtime_error(std::format("Failed with {}", GetLastError()));

    destination.resize(result);
    result = LCMapStringEx(
        LOCALE_NAME_INVARIANT,
        flag,
        source.data(),
        static_cast<int>(source.size()),
        destination.data(),
        static_cast<int>(destination.size()),
        nullptr,
        nullptr,
        0
    );
    if (result == 0)
        throw std::runtime_error(std::format("Failed with {}", GetLastError()));

    return destination;
}

std::wstring ToUpper(const std::wstring& string) { return ChangeCase(string, true); }
std::wstring ToLower(const std::wstring& string) { return ChangeCase(string, false); }

int main() try
{
    std::wcout << std::format(L"Result: {}\n", ToLower(L"HAHAHAHAH"));

    return 0;
}
catch (const std::exception& ex)
{
    std::println("{}", ex.what());
}