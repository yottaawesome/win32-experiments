export module MultiControlSamples:Raii;
import std;
import Win32;

export namespace MultiControlSamples
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
	using UniquePtr = std::unique_ptr<T, Deleter<VDeleter>>;
	template<typename T, auto VDeleter>
	using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<T>, Deleter<VDeleter>>;

	using FontUniquePtr = IndirectUniquePtr<Win32::HFONT, Win32::DeleteObject>;
}