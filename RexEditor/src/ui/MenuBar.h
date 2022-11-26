#pragma once

#include <string>
#include <map>

namespace RexEditor
{
	class MenuBar
	{
	public:

		static void RegisterMenuFunction(const std::string& menuPath, std::function<void()> toCall);

		static void DrawMenuBar();

	private:
		struct MenuItem
		{
			std::function<void()> toCall = nullptr;
			std::map<std::string, MenuItem> subItems;
		};

		static void DrawMenuItemRecursive(const std::string& name, MenuItem& item);

		inline static MenuItem s_menuRoot;
	};
}