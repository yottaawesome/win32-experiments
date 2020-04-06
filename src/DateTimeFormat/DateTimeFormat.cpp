#include <iostream>
#include <Windows.h>
#include <string>
#include <sstream>

// trim from start (in place)
void LeftTrim(std::wstring& s)
{
    s.erase(
        s.begin(),
        std::find_if(
            s.begin(),
            s.end(),
            [](int ch) { return !std::isspace(ch); }
        )
    );
}

// trim from end (in place)
void RightTrim(std::wstring& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
        }).base(), s.end());
}

// trim from both ends (in place)
void Trim(std::wstring& s)
{
    LeftTrim(s);
    RightTrim(s);
}

int main()
{
    SYSTEMTIME st;
    GetSystemTime(&st);

    // Format date buffer
    constexpr UINT dateStringLength = 9;
    wchar_t dateString[dateStringLength];
    if (GetDateFormatEx(
        LOCALE_NAME_INVARIANT,
        0,
        &st,
        L"yyyyMMdd",
        dateString,
        dateStringLength,
        nullptr
    ) == 0)
    {
        std::wcout << L"Failed" << std::endl;
    }
    else
    {
        std::wcout << dateString << std::endl;
    }

    // Format time buffer
    constexpr UINT timeStringLength = 9;
    wchar_t timeString[timeStringLength];
    if (GetTimeFormatEx(
        LOCALE_NAME_INVARIANT,
        0,
        &st,
        L"HH:mm:ss",
        timeString,
        timeStringLength
    ) == 0)
    {
        std::wcout << L"Failed" << std::endl;
    }
    else
    {
        std::wcout << timeString << std::endl;
    }

    TIME_ZONE_INFORMATION tzi;
    DWORD d = 100;
    if (d = GetTimeZoneInformation(&tzi) == TIME_ZONE_ID_INVALID)
    {
        std::wcout << L"Failed" << std::endl;
    }
    else
    {
        if(d == TIME_ZONE_ID_STANDARD)
            std::wcout << L"Standard" << std::endl;
        else if (d == TIME_ZONE_ID_DAYLIGHT)
            std::wcout << L"Daylight" << std::endl;
        else if (d == TIME_ZONE_ID_UNKNOWN)
            std::wcout << L"Unknown" << std::endl;

        std::wcout << L"POO" << std::endl;
        std::wcout << tzi.Bias << std::endl;
        std::wcout << tzi.DaylightBias << std::endl;
        std::wcout << tzi.DaylightDate.wMonth << std::endl;
        std::wcout << tzi.StandardDate.wMonth << std::endl;
    }

    std::wstringstream wss;
    wss << dateString
        << L"-"
        << timeString
        << L"."
        << st.wMilliseconds;
    DWORD actualBias = tzi.Bias * -1;
    if (actualBias >= 0)
        wss << L"+";
    wss << actualBias;
    std::wcout << wss.str() << std::endl;

}
