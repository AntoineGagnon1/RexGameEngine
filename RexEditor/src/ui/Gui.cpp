#include "REDPch.h"
#include "Gui.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>

#include "panels/PanelManager.h"

#include "GuiThemes.h"

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

	void Align(int width, Alignement align)
	{
		float alignement = 0.0f;
		if (align == Alignement::Center)
			alignement = 0.5f;
		else if (align == Alignement::Right)
			alignement = 1.0f;

		ImGuiStyle& style = ImGui::GetStyle();
		float avail = ImGui::GetContentRegionAvail().x;
		float off = (avail - width) * alignement;
		if (off > 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
	}

	void AlignVertical(int height, VerticalPos pos)
	{
		if (pos == VerticalPos::Bottom)
		{
			ImGuiStyle& style = ImGui::GetStyle();
			auto h = ImGui::GetWindowHeight();
			ImGui::SetCursorPosY(h - (height + style.WindowPadding.y));
		}
	}

	ImFont* LoadFont(const std::filesystem::path& path, int size)
	{
		auto& io = ImGui::GetIO();
		return io.Fonts->AddFontFromFileTTF(path.string().c_str(), size);
	}

#define GUI_LABEL_LEFT(func, label, code) ImGui::TextUnformatted(label); ImGui::NextColumn(); ImGui::SetNextItemWidth(-1); if(func) { code } ImGui::NextColumn();
}

namespace RexEditor
{
	// Static init
	void Imgui::Init()
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
		

		// Load the font sizes
		s_fonts.push_back(Internal::LoadFont("assets/fonts/CascadiaMono.ttf", 10));
		s_fonts.push_back(Internal::LoadFont("assets/fonts/CascadiaMono.ttf", 13));
		s_fonts.push_back(Internal::LoadFont("assets/fonts/CascadiaMono.ttf", 20));

		// Set the default to normal
		auto& io = ImGui::GetIO();
		io.FontDefault = s_fonts[(int)FontScale::Normal];
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
		RenderApi::BindFrameBuffer(RenderApi::InvalidFrameBufferID);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(::ImGui::GetDrawData());

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		
		// The context might get changed by imgui, revert back to the window
		if (RexEngine::Window::ActiveWindow() != nullptr)
			RexEngine::Window::ActiveWindow()->MakeActive();
	}


	bool Imgui::BeginWindow(const std::string& name, bool& open, WindowSetting settings)
	{
		return ImGui::Begin(name.c_str(), &open, (int)settings);
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

	void Imgui::TextInput(const std::string& label, std::string& result, size_t maxSize)
	{
		if(result.capacity() < maxSize)
			result.resize(maxSize); // Make sure the string is big enough

		ImGui::BeginColumns((label + "_col").c_str(), 2);
		GUI_LABEL_LEFT(ImGui::InputText(("##" + label).c_str(), result.data(), result.capacity()), label.c_str());
		ImGui::EndColumns();

		// Set the new size for the string, based on the content of the buffer
		result.resize(strlen(result.c_str()));
	}

	bool Imgui::Button(const std::string& label, Alignement align, VerticalPos vPos)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		float size = ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2.0f;
		
		Internal::Align(size, align);
		Internal::AlignVertical(ImGui::CalcTextSize(label.c_str()).y + style.FramePadding.y * 2.0f, vPos);

		return ImGui::Button(label.c_str());
	}

	void Imgui::IconButton(const std::string& text, const RexEngine::Texture& icon, Vector2Int size, bool& clicked, bool& doubleClicked)
	{
		auto& style = ImGui::GetStyle();
		bool selected = false;
		ImVec2 cursor = ImGui::GetCursorPos();
		ImGui::Selectable(("##selectable_" + text).c_str(), &selected, ImGuiSelectableFlags_None, { (float)size.x, (float)size.y + ImGui::CalcTextSize(text.c_str()).y + style.ItemSpacing.y});
		ImGui::SetCursorPos(cursor);

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			doubleClicked = true;
		else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
			clicked = true;

		ImGui::Image((ImTextureID)icon.GetId(), {(float)size.x, (float)size.y});

		// Centered text
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (size.x / 2.0f - ImGui::CalcTextSize(text.c_str(), NULL, false, (float)size.x).x / 2.0f));
		ImGui::TextWrapped(text.c_str(), (float)size.x);
	}

	void Imgui::SliderFloat(const std::string& label, float min, float max, float& value, int width, int precision, VerticalPos vPos)
	{
		auto& style = ImGui::GetStyle();
		Internal::AlignVertical(ImGui::GetTextLineHeight() + style.FramePadding.y * 2.0f, vPos);

		Imgui::Text(label);
		Imgui::SameLine();
		ImGui::PushItemWidth(width);
		ImGui::SliderFloat(("##" + label).c_str(), &value, min, max, ("%." + std::to_string(precision) + "f").c_str());
		ImGui::PopItemWidth();
	}

	bool Imgui::IsItemClicked(MouseButton mouseButton)
	{
		// Convert RexEngine::MouseButton to ImGui::MouseButton
		static constexpr int ImGuiButtons[] = { ImGuiMouseButton_Right, ImGuiMouseButton_Left, ImGuiMouseButton_Middle, 3, 4};
		return ImGui::IsItemClicked(ImGuiButtons[(int)mouseButton]);
	}

	void Imgui::Text(const std::string& text)
	{
		ImGui::Text(text.c_str());
	}

	bool Imgui::TreeNode(const std::string& text, bool leaf, bool openArrowOnly, bool selected)
	{
		return ImGui::TreeNodeEx(text.c_str(), 
			(leaf ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None)
		    | (openArrowOnly ? ImGuiTreeNodeFlags_OpenOnArrow : ImGuiTreeNodeFlags_None)
			| (selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None));
	}

	void Imgui::TreePop()
	{
		ImGui::TreePop();
	}

	void Imgui::SameLine()
	{
		ImGui::SameLine();
	}

	void Imgui::Space()
	{
		ImGui::Spacing();
	}

	void Imgui::Indent(float amount)
	{
		ImGui::Indent(amount);
	}

	bool Imgui::BeginTable(const std::string& name, int nbCols, Vector2Int padding)
	{
		bool value = ImGui::BeginTable(name.c_str(), nbCols);

		auto table = ImGui::GetCurrentTable();
		table->CellPaddingX = padding.x;
		table->CellPaddingY = padding.y;
		return value;
	}

	void Imgui::EndTable()
	{
		ImGui::EndTable();
	}

	void Imgui::TableNextElement()
	{
		ImGui::TableNextColumn();
	}

	void Imgui::PushFontScale(FontScale scale)
	{
		ImGui::PushFont(s_fonts[(int)scale]);
	}

	void Imgui::PopFontScale()
	{
		ImGui::PopFont();
	}
}