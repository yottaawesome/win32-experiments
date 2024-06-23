import std;
import win32;

// https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-setunhandledexceptionfilter
// https://stackoverflow.com/questions/3523716/is-there-a-function-to-convert-exception-pointers-struct-to-a-string
// https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-exception_pointers
std::string TranslateErrorCode(Win32::DWORD errorCode, std::wstring_view moduleName)
{
	Win32::HMODULE moduleToSearch =
		moduleName.empty()
		? nullptr
		: Win32::GetModuleHandleW(moduleName.data());

	const Win32::DWORD flags =
		Win32::FormatOptions::AllocateBuffer
		| Win32::FormatOptions::FromSystem
		| Win32::FormatOptions::IgnoreInserts
		| (moduleToSearch ? Win32::FormatOptions::FromHModule : 0);

	void* messageBuffer = nullptr;
	Win32::FormatMessageA(
		flags,
		moduleToSearch,
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

long __stdcall Handler(Win32::_EXCEPTION_POINTERS* info)
{
	Win32::HMODULE hm = nullptr;
	Win32::GetModuleHandleExW(Win32::FlagFromAddress, static_cast<Win32::LPCTSTR>(info->ExceptionRecord->ExceptionAddress), &hm);
	std::string modulePath(500, '\0');
	Win32::K32GetModuleFileNameExA(Win32::GetCurrentProcess(), hm, modulePath.data(), static_cast<Win32::DWORD>(modulePath.size()));
	modulePath = modulePath.c_str();

    std::println(
R"(hello from the exception handler!
	msg: {}
	code: {:#x}
	address: {:#x}
	module: {}
)", 
		TranslateErrorCode(info->ExceptionRecord->ExceptionCode, L"ntdll.dll"),
		info->ExceptionRecord->ExceptionCode,
		reinterpret_cast<unsigned long long>(info->ExceptionRecord->ExceptionAddress),
		modulePath
	);
    return Win32::Continue;
}

namespace Order
{
    enum
    {
        CallLast = 0,
        CallFirst = 1
    };
}

int main()
{
    Win32::AddVectoredExceptionHandler(Order::CallFirst, &Handler);
    int* x = nullptr;
    int y = *x;

    return 0;
}

