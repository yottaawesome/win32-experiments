export module stdlib;
export import std;
export import win32;

export
{
    consteval auto IsDebug() noexcept -> bool
    {
#ifdef _DEBUG
        return true;
#else
        return false;
#endif // DEBUG
    }

    consteval auto IsRelease() noexcept -> bool
    {
        return not IsDebug();
    }
}
