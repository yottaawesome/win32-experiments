export module common.util;
import std;
import common.win32;

export namespace Util
{
    template<typename T>
    constexpr bool AlwaysFalseT = std::false_type::value;

    template<auto T>
    constexpr bool AlwaysFalseV = std::false_type::value;

    template<auto VDeleter, auto...VStaticArgs>
    struct Deleter
    {
        using TDeleter = decltype(VDeleter);

        void operator()(auto&& objectToDelete) const noexcept 
        {
            using THandle = decltype(objectToDelete);

            if constexpr (std::invocable<TDeleter, THandle, decltype(VStaticArgs)...>)
            {
                std::invoke(
                    VDeleter,
                    objectToDelete,
                    std::forward<decltype(VStaticArgs)>(VStaticArgs)...
                );
            }
            else if constexpr (std::invocable<TDeleter, decltype(VStaticArgs)..., THandle>)
            {
                std::invoke(
                    VDeleter,
                    std::forward<decltype(VStaticArgs)>(VStaticArgs)...,
                    objectToDelete
                );
            }
            else
            {
                static_assert(AlwaysFalseT<VDeleter>, "Deleter is not invocable in any order.");
            }
        }
    };
    using HandleUniquePtr = std::unique_ptr<std::remove_pointer_t<Win32::HANDLE>, Deleter<Win32::CloseHandle>>;
    using LibraryUniquePtr = std::unique_ptr<std::remove_pointer_t<Win32::HMODULE>, Deleter<Win32::FreeLibrary>>;
    using LocalAllocUniquePtr = std::unique_ptr<std::remove_pointer_t<Win32::HLOCAL>, Deleter<Win32::LocalFree>>;

    std::string Format(Win32::DWORD errorCode, std::wstring_view moduleName = L"")
    {
        LibraryUniquePtr moduleToSearch = moduleName.empty()
            ? nullptr
            : LibraryUniquePtr(Win32::LoadLibraryW(moduleName.data()));

        constexpr auto flags =
            Win32::FormatMessageFlags::AllocateBuffer
            | Win32::FormatMessageFlags::FromSystem
            | Win32::FormatMessageFlags::IgnoreInserts;

        void* msgBuffer = nullptr;
        Win32::FormatMessageA(
            flags | (moduleToSearch ? Win32::FormatMessageFlags::FromHModule : 0),
            moduleToSearch.get(),
            errorCode,
            Win32::UserDefault,
            reinterpret_cast<Win32::LPSTR>(msgBuffer),
            0,
            nullptr
        );
        if (not msgBuffer) return std::format(
            "Failed formatting message: {} -- {}", 
            errorCode, 
            Win32::GetLastError()
        );

        LocalAllocUniquePtr ptr(msgBuffer);
        return { reinterpret_cast<char*>(msgBuffer) };
    }

    struct HResult
    {
        HResult(Win32::HRESULT hr)
            : m_hr(hr)
        { }

        Win32::HRESULT m_hr;

        operator bool() const noexcept
        {
            return Win32::HrSucceeded(m_hr);
        }

        operator Win32::HRESULT() const noexcept
        {
            return m_hr;
        }
    };

    template<auto VDeleter>
    struct GenericDeleter
    {
        void operator()(auto&& object)
        {
            VDeleter(object);
        }
    };
    using HandleDeleter = std::unique_ptr<std::remove_pointer_t<Win32::HANDLE>, GenericDeleter<Win32::CloseHandle>>;

    template<typename...TArgs>
    struct Error : public std::runtime_error
    {
        public:
        Error(std::format_string<TArgs...> fmt, TArgs&&...args, std::source_location loc = std::source_location::current())
            : std::runtime_error(
                std::format("{} in {}", 
                    std::format(fmt, std::forward<TArgs>(args)...),
                    loc.function_name()
                )
            )
        { }
    };
    template<typename...TArgs>
    Error(std::format_string<TArgs...>, TArgs&&...) -> Error<TArgs...>;

    struct GloballyUniqueID
    {
        constexpr GloballyUniqueID(Win32::GUID guid) : m_guid(guid) {}

        GloballyUniqueID()
        {
            Win32::HRESULT result = Win32::CoCreateGuid(&m_guid);
            if (Win32::HrFailed(result))
                throw Error("CoCreateGuid() failed", result);
        }

        Win32::GUID m_guid{ 0 };

        static constexpr Win32::GUID Null = Win32::NullGuid;
    };
}

export namespace Util
{
    // See https://dev.to/sgf4/strings-as-template-parameters-c20-4joh
    template<typename T>
    concept ValidCharType = std::same_as<T, char> or std::same_as<T, wchar_t>;

    template<typename T>
    concept ValidViewType = std::same_as<T, std::string_view> or std::same_as<T, std::wstring_view>;

    template<typename T>
    concept ValidStringType = std::same_as<T, std::string> or std::same_as<T, std::wstring>;

    template <ValidCharType TChar, ValidViewType TView, ValidStringType TString, size_t N>
    struct FixedString
    {
        using CharType = TChar;
        using ViewType = TView;
        using StringType = TString;

        TChar buf[N]{};

        consteval FixedString(const TChar(&arg)[N]) noexcept
        {
            std::copy_n(arg, N, buf);
        }

        // There's a consteval bug in the compiler.
        // See https://developercommunity.visualstudio.com/t/consteval-function-unexpectedly-returns/10501040
        constexpr operator const TChar* () const noexcept
        {
            return buf;
        }

        consteval TView ToView() const noexcept
        {
            return { buf };
        }

        consteval operator TView() const noexcept
        {
            return { buf };
        }

        constexpr operator TString() const noexcept
        {
            return { buf };
        }

        constexpr TString ToString() const noexcept
        {
            return { buf };
        }

        consteval bool operator==(const FixedString<TChar, TView, TString, N> str) const
        {
            return std::equal(str.buf, str.buf + N, buf);
        }

        template<ValidCharType TChar, ValidViewType TView, ValidStringType TString, std::size_t N2>
        consteval bool operator==(const FixedString<TChar, TView, TString, N2> s) const
        {
            return false;
        }

        template<std::size_t N2>
        consteval FixedString<TChar, TView, TString, N + N2 - 1> operator+(const FixedString<TChar, TView, TString, N2> str) const
        {
            TChar newchar[N + N2 - 1]{};
            std::copy_n(buf, N - 1, newchar);
            std::copy_n(str.buf, N2, newchar + N - 1);
            return newchar;
        }
    };
    template<size_t N>
    FixedString(char const (&)[N]) -> FixedString<char, std::string_view, std::string, N>;
    template<size_t N>
    FixedString(wchar_t const (&)[N]) -> FixedString<wchar_t, std::wstring_view, std::wstring, N>;

    template<ValidCharType TChar, ValidViewType TView, ValidStringType TString, std::size_t s1, std::size_t s2>
    consteval auto operator+(FixedString<TChar, TView, TString, s1> fs, const TChar(&str)[s2])
    {
        return fs + FixedString<TChar, TView, TString, s2>(str);
    }

    template<ValidCharType TChar, ValidViewType TView, ValidStringType TString, std::size_t s1, std::size_t s2>
    consteval auto operator+(const TChar(&str)[s2], FixedString<TChar, TView, TString, s1> fs)
    {
        return FixedString<s2>(str) + fs;
    }

    template<size_t N>
    using WideFixedString = FixedString<wchar_t, std::wstring_view, std::wstring, N>;
    template<size_t N>
    using NarrowFixedString = FixedString<char, std::string_view, std::string, N>;
}