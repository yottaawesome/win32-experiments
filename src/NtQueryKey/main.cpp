import win32;
import std;

struct HKeyDeleter
{
	void operator()(Win32::HKEY key) const noexcept
	{
		Win32::RegCloseKey(key);
	}
};
using HKeyUniquePtr = std::unique_ptr<Win32::HKEY__, HKeyDeleter>;

namespace std
{
	template<typename...TArgs>
	void println(std::wformat_string<TArgs...> fmt, TArgs&&...args)
	{
		std::wcout << std::format(fmt, std::forward<TArgs>(args)...) << std::endl;
	}
}

// Based on https://stackoverflow.com/a/937379/7448661
int main()
{
	Win32::HMODULE hModule = Win32::GetModuleHandleW(L"ntdll.dll");

	if (not hModule)
	{
		std::println("Failed to open ntdll.dll: {}", Win32::GetLastError());
		return 1;
	}

	auto NtQueryKey = reinterpret_cast<Win32::NtQueryKeyType>(Win32::GetProcAddress(hModule, "NtQueryKey"));
	if (not NtQueryKey)
	{
		std::println("Failed to open ntdll.dll: {}", Win32::GetLastError());
		return 1;
	}

	Win32::HKEY out;
	Win32::LSTATUS status = Win32::RegOpenKeyExW(Win32::HKLM, L"SOFTWARE\\Microsoft", 0, Win32::KeyRead, &out);
	if(status)
	{
		std::println("Failed to open key: {}", status);
		return 1;
	}
	HKeyUniquePtr ptr(out);

	Win32::DWORD size = 0;
	Win32::DWORD result = NtQueryKey(out, Win32::KeyInformationClass::KeyNameInformation, 0, 0, &size);
	if (result != Win32::NTStatus::BufferTooSmall)
	{
		std::println("NtQueryKey: {}", result);
		return 1;
	}

	std::wstring name(size / sizeof(wchar_t), '\0');
	result = NtQueryKey(out, Win32::KeyInformationClass::KeyNameInformation, name.data(), size, &size);
	if (result != Win32::NTStatus::Success)
	{
		std::println("NtQueryKey: {}", result);
		return 1;
	}

	std::println(L"Key: {}\n", name);

	return 0;
}

