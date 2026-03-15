#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>

#pragma comment(lib, "Comctl32.lib")

import std;
import formbuilder;

auto WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int) -> int
try
{
	auto icc = INITCOMMONCONTROLSEX{
		.dwSize = sizeof(INITCOMMONCONTROLSEX),
		.dwICC = ICC_STANDARD_CLASSES,
	};
	InitCommonControlsEx(&icc);

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
		MessageBoxW(e.formHwnd, L"Form submitted!", L"Submit", MB_OK | MB_ICONINFORMATION);
	});
	events.onClick(302, [](const FormDesigner::ClickEvent& e) {
		DestroyWindow(e.formHwnd);
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
	MessageBoxW(nullptr, wide.c_str(), L"Form Designer Error", MB_ICONERROR | MB_OK);
	return 1;
}
