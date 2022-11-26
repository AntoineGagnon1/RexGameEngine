#include "REDPch.h"
#include "Gui.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

namespace RexEditor
{
	// Static init
	void Imgui::Init(RexEngine::Window& window)
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

	void Imgui::Close()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void Imgui::NewFrame()
	{
		// Start a new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport();
	}

	void Imgui::RenderGui()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(::ImGui::GetDrawData());

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		
		// The context might get changed by imgui, revert back to the window
		if (s_window != nullptr)
			s_window->MakeActive();
	}


	bool Imgui::BeginWindow(const std::string& name, bool& open, WindowSetting::WindowSetting_ settings)
	{
		return ImGui::Begin(name.c_str(), &open, settings);
	}

	void Imgui::EndWindow()
	{
		ImGui::End();
	}

	Vector2Int Imgui::GetWindowSize()
	{
		auto size = ImGui::GetWindowSize();
		return {size.x, size.y};
	}

	bool Imgui::IsWindowFocused()
	{
		return ImGui::IsWindowFocused();
	}

	bool Imgui::IsWindowHovered()
	{
		return ImGui::IsWindowHovered();
	}

	void Imgui::BeginFullScreenWindow()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

		bool open = true;
		Imgui::BeginWindow("Root", open, (WindowSetting::WindowSetting_)(WindowSetting::NoDecoration | WindowSetting::NoResize | WindowSetting::MenuBar));

		ImGui::PopStyleVar();
	}

	void Imgui::DrawFullWindowTexture(const RexEngine::Texture& texture)
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

	bool Imgui::BeginMenuBar()
	{
		return ImGui::BeginMenuBar();
	}

	void Imgui::EndMenuBar()
	{
		ImGui::EndMenuBar();
	}

	bool Imgui::BeginMenu(const std::string& name, bool enabled)
	{
		return ImGui::BeginMenu(name.c_str(), enabled);
	}

	void Imgui::EndMenu()
	{
		ImGui::EndMenu();
	}

	void Imgui::MenuItem(const std::string& name, std::function<void()> toCall, bool enabled)
	{
		bool selected = false;
		ImGui::MenuItem(name.c_str(), NULL, &selected, enabled);

		if (selected)
			toCall();
	}
}