#include "REDPch.h"
#include "UI.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "panels/PanelManager.h"

#include "core/EditorEvents.h"
#include "GuiThemes.h"

namespace RexEditor::UI::Internal
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

	void PanelStateWriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
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

	// 0 = small, 1 = normal, 2 = large
	typedef std::array<ImFont*, 3> FontCollection;

	std::map<float, FontCollection> Fonts;
	std::vector<float> FontsNeeded; // Font dpi needed for the next frame

	void ImGuiInit()
	{
		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		IMGUI_CHECKVERSION();

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle(); // Style for multiple viewports :
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;

		ImGui_ImplGlfw_InitForOpenGL(RexEngine::Window::ActiveWindow()->WindowHandle(), true);
		ImGui_ImplOpenGL3_Init("#version 130");

		// Custom data in the .ini file for the state of each panel
		ImGuiSettingsHandler iniHandler;
		iniHandler.TypeName = "PanelState";
		iniHandler.TypeHash = ImHashStr("PanelState");
		iniHandler.ReadOpenFn = Internal::PanelStateReadOpen;
		iniHandler.ReadLineFn = Internal::PanelStateReadLine;
		iniHandler.WriteAllFn = Internal::PanelStateWriteAll;
		ImGui::AddSettingsHandler(&iniHandler);

		GuiThemes::EnableDark();


		// Load the default font sizes (dpiScale == 1.0f)
		auto& io = ImGui::GetIO();

		auto scale = ImGui::GetWindowDpiScale();

		FontCollection col;
		col[0] = io.Fonts->AddFontFromFileTTF("assets/fonts/CascadiaMono.ttf", 10);
		col[1] = io.Fonts->AddFontFromFileTTF("assets/fonts/CascadiaMono.ttf", 13);
		col[2] = io.Fonts->AddFontFromFileTTF("assets/fonts/CascadiaMono.ttf", 20);
		Fonts.emplace(1.0f, col);

		// Set the default to normal
		io.FontDefault = Fonts[1.0f][(int)FontScale::Normal];
	};

	void ImGuiClose()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void NewFrame()
	{
		// Load the needed fonts
		auto& io = ImGui::GetIO();
		for (float winScale : Internal::FontsNeeded)
		{
			Internal::FontCollection col;
			col[0] = io.Fonts->AddFontFromFileTTF("assets/fonts/CascadiaMono.ttf", 10 * winScale);
			col[1] = io.Fonts->AddFontFromFileTTF("assets/fonts/CascadiaMono.ttf", 13 * winScale);
			col[2] = io.Fonts->AddFontFromFileTTF("assets/fonts/CascadiaMono.ttf", 20 * winScale);
			Internal::Fonts.emplace(winScale, col);
		}

		if (Internal::FontsNeeded.size() > 0)
		{
			ImGui_ImplOpenGL3_CreateFontsTexture();
			Internal::FontsNeeded.clear();
		}

		// Start a new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport();
	}

	RE_STATIC_CONSTRUCTOR({
		EditorEvents::OnEditorStart().Register<&ImGuiInit>();
		EditorEvents::OnEditorStop().Register<&ImGuiClose>();

		EngineEvents::OnPreUpdate().Register<&NewFrame>();
	});
}

namespace RexEditor::UI
{
	void UI::PushFontScale(FontScale scale)
	{
		auto winScale = ImGui::GetWindowDpiScale();

		if (!Internal::Fonts.contains(winScale))
		{
			Internal::FontsNeeded.push_back(winScale);
			ImGui::PushFont(Internal::Fonts[1.0f][(int)scale]); // Push a default font
			return;
		}

		ImGui::PushFont(Internal::Fonts[winScale][(int)scale]);
	}

	void UI::PopFontScale()
	{
		ImGui::PopFont();
	}

	void PushFontColor(RexEngine::Color color)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, *((ImVec4*)&color));
	}

	void PopFontColor()
	{
		ImGui::PopStyleColor();
	}

	void UI::RenderUI()
	{
		RenderApi::BindFrameBuffer(RenderApi::InvalidFrameBufferID);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(::ImGui::GetDrawData());

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		// The context might get changed by imgui, revert back to the window
		if (RexEngine::Window::ActiveWindow() != nullptr)
			RexEngine::Window::ActiveWindow()->MakeActive();
	}
}