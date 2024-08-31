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
		constexpr ProcAddress(HMODULE hModule) : Module(hModule) {}
		constexpr ProcAddress()
			requires std::same_as<std::invoke_result_t<decltype(VModuleGetter)>, Win32::HMODULE>
		{ }

		template<typename...TArgs>
		constexpr auto operator()(TArgs&&...args) const noexcept(std::is_nothrow_invocable_v<TFn*, TArgs...>)
			requires std::invocable<TFn*, TArgs...>
		{
			std::call_once(
				Flag, 
				[](TFn** ptr, Win32::HMODULE classModule)
				{
					Win32::HMODULE hModule = classModule;
					if (not hModule)
						if constexpr (std::invocable<decltype(VModuleGetter)>)
							hModule = std::invoke(VModuleGetter);
						else
							hModule = VModuleGetter;
					
					*ptr = reinterpret_cast<TFn*>(Win32::GetProcAddress(hModule, VProcName.ToString().c_str()));
				},
				&FnPtr,
				Module
			);
			return std::invoke(FnPtr, std::forward<TArgs>(args)...);
		}

		mutable std::once_flag Flag;
		mutable TFn* FnPtr = nullptr;
		Win32::HMODULE Module = nullptr;
	};


	template<FixedStringA VLibPath>
	struct Library
	{
		constexpr operator HMODULE() const noexcept 
		{
			static Win32::HMODULE hModule = 
				[]{ 
					Win32::HMODULE hModule = Win32::LoadLibraryA(VLibPath.ToCharBuffer());
					if (not hModule)
						throw std::runtime_error(std::format("Failed loading library {}", VLibPath.ToView()));
					return hModule; 
				}();
			return hModule;
		}
		//Win32::HMODULE Module = nullptr; // Can use call_once
	};

	template<typename T, typename TSignature>
	concept Allowed = std::is_null_pointer_v<T> or std::is_convertible_v<T, TSignature>;

	template<FixedStringA VProcName, typename TSignature, Library VLib, auto VDefault = nullptr>
	requires Allowed<decltype(VDefault), TSignature> // Allowed VDefault syntax appears to be bugged
	struct LibraryProc
	{
		template<typename...TArgs>
		constexpr auto operator()(TArgs&&...args) const noexcept(std::is_nothrow_invocable_v<TSignature, TArgs...>)
			requires std::invocable<TSignature, TArgs...>
		{
			static TSignature proc =
				[]() -> TSignature
				{
					TSignature fn = reinterpret_cast<TSignature>(Win32::GetProcAddress(VLib, VProcName.ToCharBuffer()));
					if (fn)
						return fn;
					if constexpr (std::invocable<decltype(VDefault), TArgs...>)
						return VDefault;
					else
						throw std::runtime_error(std::format("Failed loading function {}", VProcName.ToView()));
				}();
			return std::invoke(proc, std::forward<TArgs>(args)...);
		}
	};

	struct AnAPI
	{
		static constexpr Library<"Something"> ALibrary;
		LibraryProc<"SomeProc", void(*)(), ALibrary, [](){}> Proc;
	} constexpr API;
}
