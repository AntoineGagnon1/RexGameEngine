#pragma once

#include "UIElements.h"

namespace RexEditor::UI
{
	// A helper class for menus
	template<typename...Args>
	class MenuSystem
	{
	private:
		using FuncType = std::function<void(Args...)>;
	public:
		// menuPath : SomeMenu/SubMenu/Item
		// onClick : will be called when the item is clicked
		void AddMenuItem(const std::string& menuPath, FuncType onClick)
		{
			auto path = RexEngine::StringHelper::Split(menuPath, '/');

			MenuItem* currentItem = &m_menuRoot;
			for (auto& level : path)
			{
				if (!currentItem->subItems.contains(level))
					currentItem->subItems.insert({ level, MenuItem() });

				currentItem = &currentItem->subItems[level];
			}

			currentItem->onClick = onClick;
		}

		// Draw this menu
		// This will forward the args to the click function
		void  DrawMenu(Args...args)
		{
			for (auto& subItem : m_menuRoot.subItems)
			{
				DrawMenuItemRecursive(subItem.first, subItem.second, std::forward<Args>(args)...);
			}
		}

	private:
		struct MenuItem
		{
			FuncType onClick = nullptr;
			std::map<std::string, MenuItem> subItems;
		};

		void DrawMenuItemRecursive(const std::string& name, MenuItem& item, Args...args)
		{
			if (item.subItems.empty())
			{
				if (UI::MenuItem i(name, item.onClick != nullptr); i.IsClicked()) // Draw the menu item
					item.onClick(std::forward<Args>(args)...);
			}
			else
			{
				if (UI::Menu m(name); m.IsOpen()) // Make a sub-menu
				{
					for (auto& subItem : item.subItems)
					{
						DrawMenuItemRecursive(subItem.first, subItem.second, std::forward<Args>(args)...);
					}
				}
			}
		}

		MenuItem m_menuRoot;
	};
}