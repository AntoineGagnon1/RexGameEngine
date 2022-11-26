#include "REDPch.h"
#include "MenuBar.h"

#include <src/utils/StringHelper.h>

#include "Gui.h"

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
		Imgui::BeginMenuBar();
		for (auto& subItem : s_menuRoot.subItems)
		{
			DrawMenuItemRecursive(subItem.first, subItem.second);
		}
		Imgui::EndMenuBar();
	}

	void MenuBar::DrawMenuItemRecursive(const std::string& name, MenuItem& item)
	{
		if (item.subItems.empty())
			Imgui::MenuItem(name, item.toCall, item.toCall != nullptr); // Draw the menu item
		else
		{
			if (Imgui::BeginMenu(name)) // Make a menu
			{
				for (auto& subItem : item.subItems)
				{
					DrawMenuItemRecursive(subItem.first, subItem.second);
				}
				Imgui::EndMenu();
			}
		}
	}
}