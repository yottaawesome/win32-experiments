#include <iostream>
#include <Windows.h>

// Adapted from https://stackoverflow.com/a/19941516/7448661
std::wstring GetGuidAsWString(GUID& guid)
{
    wchar_t rawGuid[64] = { 0 };
    StringFromGUID2(guid, rawGuid, 64);
    return std::wstring(rawGuid);
}

int main(int argc, char** args)
{
    GUID guid;

    if(CoCreateGuid(&guid) == S_OK)
        std::wcout << GetGuidAsWString(guid) << std::endl;
    else
        std::wcout << L"Failed GUID generation!" << std::endl;

    return 0;
}
