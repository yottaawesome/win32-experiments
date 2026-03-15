export module formbuilder:parser;
import std;
import :win32;
import :json;
import :schema;

namespace FormDesigner
{
	auto ParseControlType(const std::string& type) -> ControlType
	{
		static const auto map = std::unordered_map<std::string, ControlType>{
			{"Window",   ControlType::Window},
			{"Button",   ControlType::Button},
			{"CheckBox", ControlType::CheckBox},
			{"Label",    ControlType::Label},
			{"TextBox",  ControlType::TextBox},
			{"GroupBox", ControlType::GroupBox},
			{"ListBox",  ControlType::ListBox},
			{"ComboBox", ControlType::ComboBox},
		};

		if (auto it = map.find(type); it != map.end())
			return it->second;

		throw std::runtime_error(std::format("Unknown control type: '{}'", type));
	}

	auto ParseControl(const nlohmann::json& j) -> Control
	{
		auto control = Control{};

		control.type = ParseControlType(j.at("type").get<std::string>());

		if (j.contains("text"))
		{
			auto narrow = j["text"].get<std::string>();
			control.text = std::wstring(narrow.begin(), narrow.end());
		}

		if (j.contains("rect"))
		{
			auto& r = j["rect"];
			control.rect = Rect{
				.x = r[0].get<int>(),
				.y = r[1].get<int>(),
				.width = r[2].get<int>(),
				.height = r[3].get<int>(),
			};
		}

		if (j.contains("id"))
			control.id = j["id"].get<int>();

		if (j.contains("style"))
			control.style = j["style"].get<Win32::DWORD>();

		if (j.contains("exStyle"))
			control.exStyle = j["exStyle"].get<Win32::DWORD>();

		if (j.contains("onClick"))
			control.onClick = j["onClick"].get<std::string>();

		if (j.contains("children"))
			for (auto& child : j["children"])
				control.children.push_back(ParseControl(child));

		return control;
	}
}

export namespace FormDesigner
{
	// Parses a Form definition from a JSON string.
	auto ParseForm(const std::string& jsonText) -> Form
	{
		auto j = nlohmann::json::parse(jsonText);
		auto form = Form{};

		if (j.contains("title"))
		{
			auto narrow = j["title"].get<std::string>();
			form.title = std::wstring(narrow.begin(), narrow.end());
		}

		if (j.contains("width"))
			form.width = j["width"].get<int>();

		if (j.contains("height"))
			form.height = j["height"].get<int>();

		if (j.contains("style"))
			form.style = j["style"].get<Win32::DWORD>();

		if (j.contains("exStyle"))
			form.exStyle = j["exStyle"].get<Win32::DWORD>();

		if (j.contains("controls"))
			for (auto& control : j["controls"])
				form.controls.push_back(ParseControl(control));

		return form;
	}

	// Loads a Form definition from a JSON file on disk.
	auto LoadFormFromFile(const std::filesystem::path& path) -> Form
	{
		auto file = std::ifstream{ path };
		if (not file.is_open())
			throw std::runtime_error(std::format("Cannot open form file: '{}'", path.string()));

		auto content = std::string{
			std::istreambuf_iterator<char>{file},
			std::istreambuf_iterator<char>{}
		};

		return ParseForm(content);
	}
}
