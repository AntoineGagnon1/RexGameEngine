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

	private:
		static void DrawMenuBar(float deltaTime);

		RE_STATIC_CONSTRUCTOR({
			EditorEvents::OnUI().Register<&MenuBar::DrawMenuBar>();
		});

		inline static MenuSystem<> s_menuSystem;
	};
}