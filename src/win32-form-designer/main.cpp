#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>

import std;
import schema;
import loader;

using namespace FormDesigner;

Form BuildSampleForm()
{
	return Form{
		.title = L"Sample Form",
		.width = 400,
		.height = 300,
		.controls = {
			Control{
				.type = ControlType::Label,
				.text = L"Name:",
				.rect = { 20, 20, 60, 20 },
			},
			Control{
				.type = ControlType::TextBox,
				.text = L"",
				.rect = { 90, 18, 200, 24 },
				.id = 101,
			},
			Control{
				.type = ControlType::Label,
				.text = L"Email:",
				.rect = { 20, 55, 60, 20 },
			},
			Control{
				.type = ControlType::TextBox,
				.text = L"",
				.rect = { 90, 53, 200, 24 },
				.id = 102,
			},
			Control{
				.type = ControlType::GroupBox,
				.text = L"Options",
				.rect = { 20, 90, 270, 100 },
				.children = {
					Control{
						.type = ControlType::CheckBox,
						.text = L"Subscribe to newsletter",
						.rect = { 15, 25, 200, 20 },
						.id = 201,
					},
					Control{
						.type = ControlType::CheckBox,
						.text = L"Accept terms and conditions",
						.rect = { 15, 50, 200, 20 },
						.id = 202,
					},
				},
			},
			Control{
				.type = ControlType::Button,
				.text = L"Submit",
				.rect = { 90, 210, 100, 30 },
				.id = 301,
				.style = BS_DEFPUSHBUTTON,
			},
			Control{
				.type = ControlType::Button,
				.text = L"Cancel",
				.rect = { 200, 210, 100, 30 },
				.id = 302,
			},
		},
	};
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
	auto icc = INITCOMMONCONTROLSEX{
		.dwSize = sizeof(INITCOMMONCONTROLSEX),
		.dwICC = ICC_STANDARD_CLASSES,
	};
	InitCommonControlsEx(&icc);

	auto form = BuildSampleForm();
	auto hwnd = LoadForm(form, hInstance);
	if (not hwnd)
		return 1;

	return RunMessageLoop();
}
