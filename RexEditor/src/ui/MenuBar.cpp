#include "REDPch.h"
#include "MenuBar.h"

#include <src/utils/StringHelper.h>

#include "imgui/imgui.h"
#include "UIElements.h"

namespace RexEditor::UI
{
	void MenuBar::RegisterMenuFunction(const std::string& menuPath, std::function<void()> toCall)
	{
		s_menuSystem.AddMenuItem(menuPath, toCall);
	}

	void MenuBar::DrawMenuBar(float deltaTime)
	{
		ImGui::BeginMainMenuBar();
		s_menuSystem.DrawMenu();
		ImGui::EndMainMenuBar();
	}
}