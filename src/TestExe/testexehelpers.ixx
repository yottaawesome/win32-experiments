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

	template<FixedStringA VProcName, typename TFn, auto VModuleGetter = []{}>
	struct ProcAddress
	{
		constexpr ProcAddress()
			requires std::same_as<std::invoke_result_t<decltype(VModuleGetter)>, Win32::HMODULE>
		{
			FnPtr = reinterpret_cast<TFn*>(Win32::GetProcAddress(VModuleGetter(), VProcName.ToString().c_str()));
			if (not FnPtr)
				throw std::runtime_error(std::format("Failed loading function: {}", VProcName.ToView()));
		}

		constexpr ProcAddress(Win32::HMODULE hModule)
		{
			if (not hModule)
				throw std::runtime_error("hModule cannot be null");
			FnPtr = reinterpret_cast<TFn*>(Win32::GetProcAddress(hModule, VProcName.ToString().c_str()));
			if (not FnPtr)
				throw std::runtime_error(std::format("Failed loading function: {}", VProcName.ToView()));
		}

		constexpr ProcAddress(Win32::HMODULE hModule, TFn* alternate) noexcept
		{
			if (hModule)
				FnPtr = reinterpret_cast<TFn*>(Win32::GetProcAddress(hModule, VProcName.ToString().c_str()));
			if (not FnPtr)
				FnPtr = alternate;
			
		}

		template<typename...TArgs>
		constexpr auto operator()(TArgs&&...args) const noexcept(std::is_nothrow_invocable_v<TFn*, TArgs...>)
			requires std::invocable<TFn*, TArgs...>
		{
			return std::invoke(FnPtr, std::forward<TArgs>(args)...);
		}

		TFn* FnPtr = nullptr;
	};
}
