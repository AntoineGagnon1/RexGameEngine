#include "REDPch.h"
#include "MenuBar.h"

#include <src/utils/StringHelper.h>

#include "imgui/imgui.h"
#include "UIElements.h"

namespace RexEditor
{
	void MenuBar::RegisterMenuFunction(const std::string& menuPath, std::function<void()> toCall)
	{
		auto path = StringHelper::Split(menuPath, '/');

		MenuItem* currentItem = &s_menuRoot;
		for (auto& level : path)
		{
			if (!currentItem->subItems.contains(level))
				currentItem->subItems.insert({ level, MenuItem() });

			currentItem = &currentItem->subItems[level];
		}

		currentItem->toCall = toCall;
	}

	void MenuBar::DrawMenuBar()
	{
		ImGui::BeginMainMenuBar();
		for (auto& subItem : s_menuRoot.subItems)
		{
			DrawMenuItemRecursive(subItem.first, subItem.second);
		}
		ImGui::EndMainMenuBar();
	}

	void MenuBar::DrawMenuItemRecursive(const std::string& name, MenuItem& item)
	{
		if (item.subItems.empty())
		{
			if (UI::MenuItem i(name, item.toCall != nullptr); i.IsClicked()) // Draw the menu item
				item.toCall();
		}
		else
		{
			if (UI::Menu m(name); m.IsOpen()) // Make a sub-menu
			{
				for (auto& subItem : item.subItems)
				{
					DrawMenuItemRecursive(subItem.first, subItem.second);
				}
			}
		}
	}
}