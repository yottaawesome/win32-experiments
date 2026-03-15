module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <CommCtrl.h>

export module schema;
import std;

export namespace FormDesigner
{
	struct Rect
	{
		int x = 0;
		int y = 0;
		int width = 100;
		int height = 25;
	};

	// Maps to a subset of window styles relevant for form controls.
	enum class ControlType
	{
		Window,     // Top-level or child window
		Button,
		CheckBox,
		Label,      // STATIC control
		TextBox,    // EDIT control
		GroupBox,
		ListBox,
		ComboBox,
	};

	struct Control
	{
		ControlType type = ControlType::Window;
		std::wstring text;
		Rect rect;
		int id = 0;
		DWORD style = 0;
		DWORD exStyle = 0;
		std::vector<Control> children;
	};

	// A complete form definition: a top-level window with child controls.
	struct Form
	{
		std::wstring title = L"Untitled";
		int width = 640;
		int height = 480;
		DWORD style = WS_OVERLAPPEDWINDOW;
		DWORD exStyle = 0;
		std::vector<Control> controls;
	};

	// Returns the Win32 window class name for a given ControlType.
	constexpr const wchar_t* ClassNameFor(ControlType type) noexcept
	{
		switch (type)
		{
		case ControlType::Button:   return WC_BUTTON;
		case ControlType::CheckBox: return WC_BUTTON;
		case ControlType::Label:    return WC_STATIC;
		case ControlType::TextBox:  return WC_EDIT;
		case ControlType::GroupBox: return WC_BUTTON;
		case ControlType::ListBox:  return WC_LISTBOX;
		case ControlType::ComboBox: return WC_COMBOBOX;
		default:                    return nullptr;
		}
	}

	// Returns additional style flags implied by the ControlType.
	constexpr DWORD ImpliedStyleFor(ControlType type) noexcept
	{
		switch (type)
		{
		case ControlType::CheckBox: return BS_AUTOCHECKBOX;
		case ControlType::GroupBox: return BS_GROUPBOX;
		case ControlType::TextBox:  return WS_BORDER | ES_AUTOHSCROLL;
		case ControlType::Label:    return SS_LEFT;
		case ControlType::ListBox:  return LBS_STANDARD;
		case ControlType::ComboBox: return CBS_DROPDOWNLIST;
		default:                    return 0;
		}
	}
}
