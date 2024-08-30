export module testexehelpers;
import common;

export namespace TestExeHelpers
{
	template<size_t N, typename TChar, typename TView, typename TString>
	struct FixedString
	{
		TChar Buffer[N]{};

		consteval FixedString(const TChar(&buffer)[N])
		{
			std::copy_n(buffer, N, Buffer);
		}

		constexpr TView ToView() const noexcept
		{
			return { Buffer };
		}

		constexpr TString ToString() const noexcept
		{
			return { Buffer };
		}

		constexpr const TChar* ToCharBuffer() const noexcept
		{
			return Buffer;
		}

		constexpr size_t Size() const noexcept
		{
			return N;
		}
	};
	template<size_t N>
	FixedString(const char(&)[N]) -> FixedString<N, char, std::string_view, std::string>;
	template<size_t N>
	FixedString(const wchar_t(&)[N]) -> FixedString<N, wchar_t, std::wstring_view, std::wstring>;
	
	template<size_t N>
	using FixedStringA = FixedString<N, char, std::string_view, std::string>;
	template<size_t N>
	using FixedStringW = FixedString<N, wchar_t, std::wstring_view, std::wstring>;

	constexpr FixedStringW Test = L"This is a test";

	template<typename T>
	concept HModuleGetter = std::same_as<T, Win32::HMODULE> or std::same_as<Win32::HMODULE, std::invoke_result_t<T>>;

	template<FixedStringA VProcName, typename TFn, HModuleGetter auto VModuleGetter = []() -> Win32::HMODULE {return nullptr; } >
	struct ProcAddress
	{
		constexpr ProcAddress()
			requires std::same_as<std::invoke_result_t<decltype(VModuleGetter)>, Win32::HMODULE>
		{ }

		template<typename...TArgs>
		constexpr auto operator()(TArgs&&...args) const noexcept(std::is_nothrow_invocable_v<TFn*, TArgs...>)
			requires std::invocable<TFn*, TArgs...>
		{
			std::call_once(
				Flag, 
				[](TFn** ptr)
				{
					if constexpr (std::invocable<decltype(VModuleGetter)>)
					{
						*ptr = reinterpret_cast<TFn*>(Win32::GetProcAddress(VModuleGetter(), VProcName.ToString().c_str()));
					}
					else
					{
						*ptr = reinterpret_cast<TFn*>(Win32::GetProcAddress(VModuleGetter, VProcName.ToString().c_str()));
					}
				},
				&FnPtr
			);

			return std::invoke(FnPtr, std::forward<TArgs>(args)...);
		}

		mutable std::once_flag Flag;
		mutable TFn* FnPtr = nullptr;
	};
}
