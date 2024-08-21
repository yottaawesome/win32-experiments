export module common.util;
import std;
import common.win32;

export namespace Concepts
{
    template<typename T>
    constexpr bool AlwaysFalseT = std::false_type::value;

    template<auto T>
    constexpr bool AlwaysFalseV = std::false_type::value;
}

export namespace RAII
{
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
                static_assert(Concepts::AlwaysFalseT<VDeleter>, "Deleter is not invocable in any order.");
            }
        }
    };
    using HandleUniquePtr = std::unique_ptr<std::remove_pointer_t<Win32::HANDLE>, Deleter<Win32::CloseHandle>>;
    using LibraryUniquePtr = std::unique_ptr<std::remove_pointer_t<Win32::HMODULE>, Deleter<Win32::FreeLibrary>>;
    using LocalAllocUniquePtr = std::unique_ptr<std::remove_pointer_t<Win32::HLOCAL>, Deleter<Win32::LocalFree>>;

    template<auto VDeleter>
    struct GenericDeleter
    {
        void operator()(auto&& object)
        {
            VDeleter(object);
        }
    };
    using HandleDeleter = std::unique_ptr<std::remove_pointer_t<Win32::HANDLE>, GenericDeleter<Win32::CloseHandle>>;
}

export namespace Error
{
    std::string TranslateErrorCode(Win32::DWORD errorCode)
    {
        constexpr Win32::DWORD flags =
            Win32::FormatMessageFlags::AllocateBuffer
            | Win32::FormatMessageFlags::FromSystem
            | Win32::FormatMessageFlags::IgnoreInserts;

        void* messageBuffer = nullptr;
        Win32::FormatMessageA(
            flags,
            nullptr,
            errorCode,
            0,
            reinterpret_cast<char*>(&messageBuffer),
            0,
            nullptr
        );
        if (not messageBuffer)
        {
            const auto lastError = Win32::GetLastError();
            return std::format("FormatMessageA() failed on code {} with error {}", errorCode, lastError);
        }

        std::string msg(static_cast<char*>(messageBuffer));
        // This should never happen
        // See also https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-raisefailfastexception
        if (Win32::LocalFree(messageBuffer))
            std::abort();

        std::erase_if(msg, [](const char x) { return x == '\n' || x == '\r'; });
        return msg;
    }

    struct BasicError : public std::runtime_error
    {
        BasicError(std::string_view s) : std::runtime_error(std::string{s})
        { }

        template<typename...TArgs>
        BasicError(std::format_string<TArgs...> fmt, TArgs&&...args)
            : std::runtime_error(Format(fmt, std::forward<TArgs>(args)...))
        { }

        template<typename...TArgs>
        constexpr std::string Format(std::format_string<TArgs...> fmt, TArgs&&...args)
        {
            return std::format("{}", std::format(fmt, std::forward<TArgs>(args)...));
        }
    };

    struct Win32Error : public BasicError
    {
        Win32Error(Win32::DWORD errorCode, std::string_view s) : BasicError(s)
        { }

        template<typename...TArgs>
        Win32Error(Win32::DWORD errorCode, std::format_string<TArgs...> fmt, TArgs&&...args)
            : BasicError(Format(errorCode, fmt, std::forward<TArgs>(args)...))
        { }

        DWORD ErrorCode() const noexcept
        {
            return m_errorCode;
        }

        private:
            template<typename...TArgs>
            constexpr std::string Format(Win32::DWORD errorCode, std::format_string<TArgs...> fmt, TArgs&&...args)
            {
                return std::format("{}: {}", std::format(fmt, std::forward<TArgs>(args)...), TranslateErrorCode(errorCode));
            }

        private:
            DWORD m_errorCode = 0;
    };

    struct COMError : public BasicError
    {
        COMError(Win32::HRESULT errorCode, std::string_view s) : BasicError(s)
        { }

        template<typename...TArgs>
        COMError(Win32::HRESULT errorCode, std::format_string<TArgs...> fmt, TArgs&&...args)
            : BasicError(Format(errorCode, fmt, std::forward<TArgs>(args)...))
        { }

        HRESULT ErrorCode() const noexcept
        {
            return m_errorCode;
        }

    private:
        template<typename...TArgs>
        constexpr std::string Format(Win32::HRESULT errorCode, std::format_string<TArgs...> fmt, TArgs&&...args)
        {
            return std::format("{}: {}", std::format(fmt, std::forward<TArgs>(args)...), TranslateErrorCode(errorCode));
        }

    private:
        HRESULT m_errorCode = 0;
    };

    constexpr void CheckHResult(Win32::HRESULT hr, std::string_view msg = "HRESULT failure.", auto&&...args)
    {
        if (Win32::HrFailed(hr))
            throw COMError(hr, msg, std::forward<decltype(args)>(args)...);
    }
}

export namespace Util
{
    template<typename TType, typename TCom>
    struct Mixin
    {
        long GetCount() const
            requires std::invocable<decltype(&TCom::get_Count), typename TType::Underlying*, long*>
        {
            const TType* self = static_cast<const TType*>(this);
            if (not self->Get())
                throw std::runtime_error("Cannot get_Count() on null.");
            long count;
            Error::CheckHResult(self->Get()->get_Count(&count), "get_Count() failed!");
            return count;
        }

        long GetTotalHistoryCount() const
            requires std::invocable<decltype(&TCom::GetTotalHistoryCount), typename TType::Underlying*, long*>
        {
            const TType* self = static_cast<const TType*>(this);
            if (not self->Get())
                throw std::runtime_error("Cannot GetTotalHistoryCount() on null.");
            long count;
            Error::CheckHResult(self->Get()->GetTotalHistoryCount(&count), "GetTotalHistoryCount() failed!");
            return count;
        }
    };

    template<typename TCom>
    struct ComPtr : public Mixin<ComPtr<TCom>, TCom>
    {
        using Underlying = TCom;
        #pragma region Constructors
        ~ComPtr() { Release(); }

        // Constructores
        ComPtr() = default;

        ComPtr(const ComPtr<TCom>& other) { Copy(other); }

        ComPtr(ComPtr<TCom>&& other) { Move(other); };

        ComPtr(TCom* ptr) : m_ptr(ptr)
        {
            if (not m_ptr)
                throw std::runtime_error("Cannot pass null.");
        }
        #pragma endregion

        #pragma region Operators
        ComPtr<TCom>& operator=(const ComPtr<TCom>& other) { return Copy(other); }

        ComPtr<TCom>& operator=(ComPtr<TCom>&& other) { return Move(other); }

        TCom* operator->() const noexcept { return m_ptr; }

        operator bool() const noexcept { return m_ptr; }
        #pragma endregion

        #pragma region Public functions
        TCom** ReleaseAndGetAddress() noexcept
        {
            Release();
            return &m_ptr;
        }

        void Release()
        {
            if (m_ptr)
            {
                m_ptr->Release();
                m_ptr = nullptr;
            }
        }

        TCom* Get() const noexcept { return m_ptr; }

        void Do(auto&& fn, auto&&...args)
        {
            Win32::HRESULT hr = std::invoke(fn, m_ptr, std::forward<decltype(args)>(args)...);
            Error::CheckHResult(hr, "Function invocation failed.");
        }
        #pragma endregion

        #pragma region Private
        private:
        ComPtr<TCom>& Move(ComPtr<TCom>& other)
        {
            Release();
            m_ptr = other->m_ptr;
            other->m_ptr = nullptr;
            return *this;
        }

        ComPtr<TCom>& Copy(ComPtr<TCom>& other)
        {
            Release();
            if (other->m_ptr)
            {
                m_ptr = other->m_ptr;
                m_ptr->AddRef();
            }
            return *this;
        }

        private:
        TCom* m_ptr = nullptr;
        #pragma endregion
    };

    std::string Format(Win32::DWORD errorCode, std::wstring_view moduleName = L"")
    {
        RAII::LibraryUniquePtr moduleToSearch = moduleName.empty()
            ? nullptr
            : RAII::LibraryUniquePtr(Win32::LoadLibraryW(moduleName.data()));

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

        RAII::LocalAllocUniquePtr ptr(msgBuffer);
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

    struct GloballyUniqueID
    {
        constexpr GloballyUniqueID(Win32::GUID guid) : m_guid(guid) {}

        GloballyUniqueID()
        {
            Win32::HRESULT result = Win32::CoCreateGuid(&m_guid);
            if (Win32::HrFailed(result))
                throw Error::COMError(result, "CoCreateGuid() failed");
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

export bool operator==(const Win32::GUID g1, const Win32::GUID g2)
{
    return Win32::IsEqualGUID(g1, g2);
}