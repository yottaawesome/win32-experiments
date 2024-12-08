export module stdlib;
export import std;
export import win32;

export
{
    constexpr bool IsDebug =
#ifdef _DEBUG
        true;
#else
        false;
#endif // DEBUG

    constexpr bool IsRelease = not IsDebug;
}
