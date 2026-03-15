#pragma comment(lib, "Comctl32.lib")

import std;
import formbuilder;

auto __stdcall wWinMain(Win32::HINSTANCE hInstance, Win32::HINSTANCE, Win32::LPWSTR lpCmdLine, int) -> int
try
{
	auto icc = Win32::INITCOMMONCONTROLSEX{
		.dwSize = sizeof(Win32::INITCOMMONCONTROLSEX),
		.dwICC = Win32::Icc_StandardClasses,
	};
	Win32::InitCommonControlsEx(&icc);

	auto path = std::filesystem::path{};
	if (lpCmdLine and lpCmdLine[0] != L'\0')
	{
		path = lpCmdLine;
	}
	else
	{
		path = L"sample-form.json";
	}

	auto form = FormDesigner::LoadFormFromFile(path);

	auto events = FormDesigner::EventMap{};
	events.onClick(301, [](const FormDesigner::ClickEvent& e) {
		Win32::MessageBoxW(e.formHwnd, L"Form submitted!", L"Submit", Win32::Mb_Ok | Win32::Mb_IconInformation);
	});
	events.onClick(302, [](const FormDesigner::ClickEvent& e) {
		Win32::DestroyWindow(e.formHwnd);
	});

	auto hwnd = FormDesigner::LoadForm(form, hInstance, events);
	if (not hwnd)
		return 1;

	return FormDesigner::RunMessageLoop();
}
catch (const std::exception& ex)
{
	auto msg = std::string{ "Failed to load form:\n" } + ex.what();
	auto wide = std::wstring(msg.begin(), msg.end());
	Win32::MessageBoxW(nullptr, wide.c_str(), L"Form Designer Error", Win32::Mb_IconError | Win32::Mb_Ok);
	return 1;
}
