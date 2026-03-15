module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module formbuilder:events;
import std;

export namespace FormDesigner
{
	// Type-safe representation of a button click from WM_COMMAND/BN_CLICKED.
	struct ClickEvent
	{
		int controlId;
		HWND controlHwnd;
		HWND formHwnd;
	};

	// Maps control IDs to strongly-typed event handlers.
	struct EventMap
	{
		void onClick(int controlId, std::function<void(const ClickEvent&)> handler)
		{
			m_clickHandlers[controlId] = std::move(handler);
		}

		auto findClickHandler(int controlId) const -> const std::function<void(const ClickEvent&)>*
		{
			if (auto it = m_clickHandlers.find(controlId); it != m_clickHandlers.end())
				return &it->second;
			return nullptr;
		}

	private:
		std::unordered_map<int, std::function<void(const ClickEvent&)>> m_clickHandlers;
	};
}
