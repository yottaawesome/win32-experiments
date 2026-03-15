export module formbuilder:schema;
import std;
import :win32;

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
		Win32::DWORD style = 0;
		Win32::DWORD exStyle = 0;
		std::string onClick;
		std::vector<Control> children;
	};

	// A complete form definition: a top-level window with child controls.
	struct Form
	{
		std::wstring title = L"Untitled";
		int width = 640;
		int height = 480;
		Win32::DWORD style = Win32::Styles::OverlappedWindow;
		Win32::DWORD exStyle = 0;
		std::vector<Control> controls;
	};

	// Returns the Win32 window class name for a given ControlType.
	constexpr auto ClassNameFor(ControlType type) noexcept -> const wchar_t*
	{
		switch (type)
		{
		case ControlType::Button:   return Win32::Controls::Button;
		case ControlType::CheckBox: return Win32::Controls::Button;
		case ControlType::Label:    return Win32::Controls::Static;
		case ControlType::TextBox:  return Win32::Controls::Edit;
		case ControlType::GroupBox: return Win32::Controls::Button;
		case ControlType::ListBox:  return Win32::Controls::ListBox;
		case ControlType::ComboBox: return Win32::Controls::ComboBox;
		default:                    return nullptr;
		}
	}

	// Returns additional style flags implied by the ControlType.
	constexpr auto ImpliedStyleFor(ControlType type) noexcept -> Win32::DWORD
	{
		switch (type)
		{
		case ControlType::CheckBox: return Win32::Styles::AutoCheckBox;
		case ControlType::GroupBox: return Win32::Styles::GroupBox;
		case ControlType::TextBox:  return Win32::Styles::Border | Win32::Styles::EditAutoHScroll;
		case ControlType::Label:    return Win32::Styles::StaticLeft;
		case ControlType::ListBox:  return Win32::Styles::ListBoxStandard;
		case ControlType::ComboBox: return Win32::Styles::ComboBoxDropDownList;
		default:                    return 0;
		}
	}
}
