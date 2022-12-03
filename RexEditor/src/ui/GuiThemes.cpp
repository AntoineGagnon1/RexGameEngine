#include "REDPch.h"
#include "GuiThemes.h"

#include <imgui/imgui.h>

void RexEditor::GuiThemes::EnableDark()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.FrameRounding = 8.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;
    style.WindowPadding = { 8,8 };
    style.FramePadding = { 4,3 };
    style.ItemSpacing = { 8,4 };
    style.ScrollbarSize = 18;
    style.GrabMinSize = 12;
    style.WindowRounding = 8.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 5.0f;
    style.WindowTitleAlign = { 0.5f, 0.5f };
    
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.34f, 0.34f, 0.34f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.44f, 0.44f, 0.44f, 0.54f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.44f, 0.44f, 0.44f, 0.54f);
    colors[ImGuiCol_Separator] = ImVec4(0.44f, 0.44f, 0.44f, 0.54f);
    colors[ImGuiCol_Tab] = ImVec4(0.28f, 0.28f, 0.28f, 0.86f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
}
