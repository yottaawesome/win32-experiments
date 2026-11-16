export module CpuId:Raii;
import std;
import Win32;

export namespace Raii
{
    template<auto VDeleteFn>
    struct Deleter
    {
        static constexpr void operator()(auto handle) noexcept
        {
            VDeleteFn(handle);
        }
    };
    template<typename T, auto VDeleter>
    using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<T>, Deleter<VDeleter>>;
    using FontUniquePtr = IndirectUniquePtr<Win32::HFONT, Win32::DeleteObject>;
}