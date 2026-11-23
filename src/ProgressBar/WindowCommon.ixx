export module ProgressBarSamples:WindowCommon;
import std;
import Win32;

export namespace ProgressBarSamples
{
	struct CreateWindowArgs
	{
		std::wstring ClassName = L"";
		std::wstring Title = L"";
		Win32::DWORD ExtendedStyle = 0;
		Win32::DWORD Style = 0;
		int X = 0;
		int Y = 0;
		unsigned Width = 0;
		unsigned Height = 0;
		Win32::HWND Parent = nullptr;
	};

	struct CreateControlArgs
	{
		unsigned ControlId = 0;
		std::wstring ClassName = L"";
		std::wstring Title = L"";
		Win32::DWORD ExtendedStyle = 0;
		Win32::DWORD Style = 0;
		int X = 0;
		int Y = 0;
		unsigned Width = 0;
		unsigned Height = 0;
		Win32::HWND Parent = nullptr;
	};

	auto NextControlId() -> unsigned
	{
		static auto s_nextControlId = 1u;
		return s_nextControlId++;
	}
}