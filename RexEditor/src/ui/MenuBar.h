#pragma once

#include <string>
#include <map>

#include "MenuSystem.h"

namespace RexEditor::UI
{
	// The main menu bar
	class MenuBar
	{
	public:

		static void RegisterMenuFunction(const std::string& menuPath, std::function<void()> toCall);

		static void DrawMenuBar();

	private:

		inline static MenuSystem<> s_menuSystem;
	};
}