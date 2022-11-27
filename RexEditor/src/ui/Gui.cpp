#include "REDPch.h"
#include "Gui.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>

#include "PanelManager.h"

namespace RexEditor::Internal
{
	// Manually add a bool in the .ini file to restore the open/closed state of each panel
	struct PanelStateEntry
	{
		std::string name;
		bool open;
	};

	void* PanelStateReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name) 
	{ 
		PanelStateEntry* ptr = new PanelStateEntry();
		ptr->name = name;
		return ptr;
	}

	void PanelStateReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
	{
		PanelStateEntry* ptr = (PanelStateEntry*)entry;
		int open = 0;
		if (sscanf(line, "Open=%d", &open) == 1) 
		{
			auto panel = PanelManager::GetPanel(ptr->name);
			if (panel)
				panel->Show(open != 0);

			delete ptr; // Looks like ImGui does not delete it ?
		}
	}

	static void PanelStateWriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
	{
		buf->reserve(buf->size() + sizeof(PanelStateEntry) * PanelManager::PanelCount());

		// Write to text buffer
		for (size_t i = 0; i < PanelManager::PanelCount(); i++)
		{
			Panel* panel = PanelManager::GetPanel(i);
			buf->appendf("[%s][%s]\n", handler->TypeName, panel->Title().c_str());
			buf->appendf("Open=%d\n", panel->IsVisible());
			buf->append("\n");
		}
	}
}

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

		// Custom data in the .ini file for the state of each panel
		ImGuiSettingsHandler iniHandler;
		iniHandler.TypeName = "PanelState";
		iniHandler.TypeHash = ImHashStr("PanelState");
		iniHandler.ReadOpenFn = Internal::PanelStateReadOpen;
		iniHandler.ReadLineFn = Internal::PanelStateReadLine;
		iniHandler.WriteAllFn = Internal::PanelStateWriteAll;
		ImGui::AddSettingsHandler(&iniHandler);
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

	bool Imgui::BeginMainMenuBar()
	{
		return ImGui::BeginMainMenuBar();
	}

	void Imgui::EndMainMenuBar()
	{
		ImGui::EndMainMenuBar();
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