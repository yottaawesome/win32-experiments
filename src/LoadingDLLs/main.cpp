import std;
import win32;

struct Deleter
{
	constexpr void operator()(this auto, Win32::HMODULE hModule) noexcept
	{
		if consteval 
		{ 
			delete hModule; 
			return; 
		}
		Win32::FreeLibrary(hModule);
	}
};

using LibraryUniquePtr = std::unique_ptr<std::remove_pointer_t<Win32::HMODULE>, Deleter>;

struct DynamicLinkLibrary
{
	constexpr DynamicLinkLibrary(Win32::HMODULE hModule)
		: Module(LibraryUniquePtr(hModule))
	{ }

	constexpr DynamicLinkLibrary(std::string_view path) 
	{
		Module =
			[path]
			{
				if (path.empty())
					throw std::runtime_error("Empty path");
				if consteval
					{ return LibraryUniquePtr(new Win32::HINSTANCE__{}); }
				return LibraryUniquePtr(Win32::LoadLibraryA(path.data()));
			}();
		if (not Module)
			throw std::runtime_error("Failed loading DLL");
	}

	constexpr void Close(this auto& self) noexcept
	{
		self.Module.reset();
	}

	constexpr operator bool(this auto& self) noexcept
	{
		return self.Module != nullptr;
	}

	constexpr auto Resolve(this const auto& self, std::string_view name) noexcept
		-> Win32::FARPROC
	{
		if (not self.Module)
			return nullptr;
		if consteval
			{ return name == "found" ? []->Win32::INT_PTR { return {}; } : nullptr; }
		return Win32::GetProcAddress(self.Module.get(), name.data());
	}

	LibraryUniquePtr Module;
};

static_assert(
	[] consteval 
	{
		DynamicLinkLibrary dll(new Win32::HINSTANCE__());
		if (not dll)
			return false;
		
		if (auto fn = dll.Resolve("found"); not fn)
			return false;
		if (auto fn = dll.Resolve("nonexistent"); fn)
			return false;

		dll.Close();
		if (dll)
			return false;

		return true;
	}());

auto main() -> int
try
{
	return 0;
}
catch (const std::exception& ex)
{
	std::println("Failed: {}", ex.what());
	return 1;
}