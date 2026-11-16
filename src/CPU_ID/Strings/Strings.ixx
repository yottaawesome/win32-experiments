export module CpuId:Strings;
import std;
import Win32;
import :Error;

export namespace Strings
{
    template<size_t N, typename TChar>
    struct FixedString
    {
        using TView = std::basic_string_view<TChar>;
        using TString = std::basic_string<TChar>;

        TChar value[N];
        constexpr FixedString(const TChar(&str)[N])
        {
            std::copy_n(str, N, value);
        }
        [[nodiscard]]
        constexpr auto ToView() const noexcept -> TView
        {
            return TView{ value, N - 1 };
        }
        [[nodiscard]]
        constexpr auto ToString() const noexcept -> TString
        {
            return TString{ value, N - 1 };
        }
    };

    template<size_t N>
    FixedString(const char(&)[N]) -> FixedString<N, char>;
    template<size_t N>
    FixedString(const wchar_t(&)[N]) -> FixedString<N, wchar_t>;

    auto Convert(const std::string& str) -> std::wstring
    {
        if (str.empty())
            return {};
        auto sizeNeeded = Win32::MultiByteToWideChar(
            Win32::CodePage::Utf8,
            0,
            str.data(),
            static_cast<int>(str.size()),
            nullptr,
            0
        );
        if (sizeNeeded <= 0)
            throw Error::Win32Error{ Win32::GetLastError(), "MultiByteToWideChar() failed [1]" };

        auto wstr = std::wstring(sizeNeeded, '\0');
        auto result = Win32::MultiByteToWideChar(
            Win32::CodePage::Utf8,
            0,
            str.data(),
            static_cast<int>(str.size()),
            wstr.data(),
            sizeNeeded
        );
        if (result <= 0)
            throw Error::Win32Error{ Win32::GetLastError(), "MultiByteToWideChar() failed [2]" };

        return wstr;
    }
}