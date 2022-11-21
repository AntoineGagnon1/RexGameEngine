#include "REDPch.h"
#include "Gui.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

namespace RexEditor
{
	// Static init
	void Gui::Init(RexEngine::Window& window)
	{
		s_window = &window;

		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		IMGUI_CHECKVERSION();

		ImGui::StyleColorsDark();
		
		ImGuiStyle& style = ImGui::GetStyle(); // Style for multiple viewports :
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		
		ImGui_ImplGlfw_InitForOpenGL(window.WindowHandle(), true);
		ImGui_ImplOpenGL3_Init("#version 130");
	};

	void Gui::Close()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void Gui::NewFrame()
	{
		// Start a new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport();
	}

	void Gui::RenderGui()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		
		// The context might get changed by imgui, revert back to the window
		if (s_window != nullptr)
			s_window->MakeActive();
	}


	bool Gui::BeginWindow(const std::string& name, bool& open, WindowSetting::WindowSetting_ settings)
	{
		return ImGui::Begin(name.c_str(), &open, settings);
	}

	void Gui::EndWindow()
	{
		ImGui::End();
	}

	Vector2Int Gui::GetWindowSize()
	{
		auto size = ImGui::GetWindowSize();
		return {size.x, size.y};
	}

	bool Gui::IsWindowFocused()
	{
		return ImGui::IsWindowFocused();
	}

	bool Gui::IsWindowHovered()
	{
		return ImGui::IsWindowHovered();
	}

	void Gui::DrawFullWindowTexture(const RexEngine::Texture& texture)
	{
		auto min = ImGui::GetWindowContentRegionMin();
		auto max = ImGui::GetWindowContentRegionMax();
		auto winPos = ImGui::GetWindowPos();

		min.x += winPos.x;
		min.y += winPos.y;
		max.x += winPos.x;
		max.y += winPos.y;

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddImage((ImTextureID)texture.GetId(),
			min,
			max,
			ImVec2(0, 1),
			ImVec2(1, 0));
	}

}