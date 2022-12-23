#include "REDPch.h"
#include "UIElements.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "SystemDialogs.h"
#include "DragDrop.h"
#include "UI.h"

// Widget sizes : https://github.com/ocornut/imgui/issues/3714

namespace RexEditor::UI::Internal
{
	ImVec2 VecConvert(const Vector2& from)
	{
		return *(ImVec2*)((Vector2*)&from);
	}

	Vector2 VecConvert(const ImVec2& from)
	{
		return *(Vector2*)((ImVec2*)&from);
	}

	bool Contains(AnchorPos source, AnchorPos filter)
	{
		return ((int)source & (int)filter) != 0;
	}

	// Convert RexEngine::MouseButton to ImGui::MouseButton
	constexpr int ImGuiButtons[] = { ImGuiMouseButton_Right, ImGuiMouseButton_Left, ImGuiMouseButton_Middle, 3, 4 };

	std::string GetVisibleLabel(const std::string& label)
	{
		return label.substr(0, label.find("##"));
	}

	// Setup for a simple full width input field
	// usage : 
	// SetupInput(label);
	// ImGui::Input...();
	void SetupInput(const std::string& label)
	{
		auto& style = ImGui::GetStyle();
		const Vector2 size{ ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight() };

		// Label (to the left of the box)
		Anchor::SetCursorPos(size);
		auto visibleLabel = GetVisibleLabel(label);
		if (visibleLabel.size() > 0)
		{
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(visibleLabel.c_str());
			ImGui::SameLine();
		}
		// Text box
		ImGui::SetNextItemWidth(-1); // Full width
	}
}


namespace RexEditor::UI
{
	void Hoverable::CacheHovered()
	{
		m_hovered = ImGui::IsItemHovered();
	}

	bool Clickable::IsClicked(RexEngine::MouseButton mouseButton, MouseAction action) const
	{
		if(action == MouseAction::Clicked)
			return(IsHovered() && ImGui::IsMouseClicked(Internal::ImGuiButtons[(int)mouseButton]));
		else
			return(IsHovered() && ImGui::IsMouseReleased(Internal::ImGuiButtons[(int)mouseButton]));
	}

	bool Clickable::IsDoubleClicked(RexEngine::MouseButton mouseButton) const
	{
		return(IsHovered() && ImGui::IsMouseDoubleClicked(Internal::ImGuiButtons[(int)mouseButton]));
	}

	void UI::SameLine()
	{
		ImGui::SameLine();
	}

	void UI::Separator()
	{
		Anchor::SetCursorPos({ ImGui::GetContentRegionAvail().x, ImGui::GetCurrentWindow()->DC.CurrLineSize.y});
		ImGui::Separator();
	}

	void EmptyLine()
	{
		Anchor::AddOffset({0, ImGui::GetTextLineHeight()});
	}


	WindowSetting operator|(WindowSetting lhs, WindowSetting rhs)
	{
		using T = std::underlying_type_t <WindowSetting>;
		return static_cast<WindowSetting>(static_cast<T>(lhs) | static_cast<T>(rhs));
	}
	WindowSetting& operator|=(WindowSetting& lhs, WindowSetting rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}


	Window::Window(const std::string& title, bool* open, WindowSetting settings)
	{
		m_visible = ImGui::Begin(title.c_str(), open, (int)settings);
		m_hovered = ImGui::IsWindowHovered();
		UI::PushFontScale(FontScale::Normal);
	}

	Window::~Window()
	{
		UI::PopFontScale();
		ImGui::End();
	}

	void Window::DrawFullWindowTexture(const Texture& texture)
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

	Vector2 Window::Size() const
	{
		return Internal::VecConvert(ImGui::GetWindowSize());
	}

	bool Window::IsFocused() const
	{
		return ImGui::IsWindowFocused();
	}

	Anchor::Anchor(AnchorPos anchor)
		: m_anchor(anchor), m_currentPos(0.0f, 0.0f)
	{
		s_currentAnchors.push(this);
		m_startPos = Internal::VecConvert(ImGui::GetCursorPos());

		// Setup the current pos
		auto& style = ImGui::GetStyle();
		auto region = ImGui::GetContentRegionAvail();

		if (Internal::Contains(m_anchor, AnchorPos::Bottom))
			m_currentPos.y = ImGui::GetCursorPosY() + region.y;
		else if (Internal::Contains(m_anchor, AnchorPos::Top))
			m_currentPos = ImGui::GetCursorPosY() + style.WindowPadding.y;
		else // Middle
			m_currentPos.y = m_startPos.y;
	}

	Anchor::~Anchor()
	{
		RE_ASSERT(s_currentAnchors.top() == this, "Mismatched UI::Anchor creation / deletion !");
		s_currentAnchors.pop();

		if(Internal::Contains(m_anchor, AnchorPos::Top) || Internal::Contains(m_anchor, AnchorPos::Bottom))
			ImGui::SetCursorPos(Internal::VecConvert(m_startPos));
	}

	void Anchor::AddOffset(Vector2 offset)
	{
		Anchor* current = s_currentAnchors.size() > 0 ? s_currentAnchors.top() : nullptr;

		if (current == nullptr || current->m_anchor == AnchorPos::Default)
			ImGui::SetCursorPos({ ImGui::GetCursorPosX() + offset.x, ImGui::GetCursorPosY() + offset.y });
		else
			current->m_currentPos = current->m_currentPos + offset;
	}

	void Anchor::SetCursorPos(Vector2 size, bool moveCursor)
	{
		Anchor* current = s_currentAnchors.size() > 0 ? s_currentAnchors.top() : nullptr;

		if (current == nullptr || current->m_anchor == AnchorPos::Default)
			return; // Skip if there is no anchor active

		auto& style = ImGui::GetStyle();
		auto region = ImGui::GetContentRegionAvail();

		// Add / Remove the size of the element
		if (Internal::Contains(current->m_anchor, AnchorPos::Top))
			current->m_currentPos += size.y;
		else if (Internal::Contains(current->m_anchor, AnchorPos::Bottom))
			current->m_currentPos -= size.y;
		// Horizontal anchors
		if (Internal::Contains(current->m_anchor, AnchorPos::Right))
			current->m_currentPos.x = region.x - size.x + style.WindowPadding.x;
		else if (Internal::Contains(current->m_anchor, AnchorPos::Center))
			current->m_currentPos.x = (region.x - size.x + style.WindowPadding.x * 2) * 0.5f;
		else // Left
			current->m_currentPos.x = style.WindowPadding.x;

		// Set the position
		ImGui::SetCursorPos(Internal::VecConvert(current->m_currentPos));

		if (!moveCursor)
			return;

		// Calculate the next position
		if (Internal::Contains(current->m_anchor, AnchorPos::Top))
			current->m_currentPos.y += style.ItemSpacing.y; // The size is already accounted for
		else if (Internal::Contains(current->m_anchor, AnchorPos::Bottom))
			current->m_currentPos.y -= style.ItemSpacing.y; // The size is already accounted for
		else // Center
			current->m_currentPos.y += size.y + style.ItemSpacing.y;
	}

	TextInput::TextInput(const std::string& label, size_t maxSize, std::string& value, bool readOnly)
		: Input(value)
	{
		if (m_value.capacity() < maxSize)
			m_value.resize(maxSize); // Make sure the string is big enough

		Internal::SetupInput(label);
		m_changed = ImGui::InputText(("##" + label).c_str(), m_value.data(), m_value.capacity(), readOnly ? ImGuiInputTextFlags_ReadOnly : 0);
		CacheHovered();
		// Set the new size for the string, based on the content of the buffer
		m_value.resize(strlen(m_value.c_str()));
	}


	Vector3Input::Vector3Input(const std::string& label, Vector3& value)
		: Input(value)
	{
		Internal::SetupInput(label);
		m_changed = ImGui::InputFloat3(("##" + label).c_str(), &m_value.x);
		CacheHovered();
	}

	Vector4Input::Vector4Input(const std::string& label, Vector4& value)
		: Input(value)
	{
		Internal::SetupInput(label);
		m_changed = ImGui::InputFloat4(("##" + label).c_str(), &m_value.x);
		CacheHovered();
	}

	Matrix4Input::Matrix4Input(const std::string& label, Matrix4& value)
		: Input(value)
	{
		std::string space(Internal::GetVisibleLabel(label).size(), ' '); // Hack to align the Inputs
		Vector4Input v0(label, value[0]);
		Vector4Input v1(space + "##1" + label, value[1]);
		Vector4Input v2(space + "##2" + label, value[2]);
		Vector4Input v3(space + "##3" + label, value[3]);

		m_hovered = v0.IsHovered() || v1.IsHovered() || v2.IsHovered() || v3.IsHovered();
		m_changed = v0.HasChanged() || v1.HasChanged() || v2.HasChanged() || v3.HasChanged();
	}

	std::filesystem::path Internal::AssetInputUI(const std::string& label, const std::vector<std::string>& filter, const Guid& currentGuid, bool& hovered)
	{
		auto& style = ImGui::GetStyle();
		auto& io = ImGui::GetIO();
		const Vector2 size{ ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight() };

		// Get the name of this asset (if any)
		// The name is the filename (stem)
		std::string name = "";
		auto path = AssetManager::GetAssetPathFromGuid(currentGuid);
		if (path.has_stem())
			name = path.stem().string(); // Remove the file extension

		Internal::SetupInput(label);
		ImGui::InputText(("##" + label).c_str(), name.data(), name.length(), ImGuiInputTextFlags_ReadOnly);
		hovered = ImGui::IsItemHovered();
		auto payload = UI::DragDrop::Target<std::filesystem::path>("Asset" + filter[0]); // The first element in filter is always type.name
		if (payload)
		{
			return *payload;
		}


		if (ImGui::IsItemClicked()) // Clicked the text, open the file selector
		{
			auto path = SystemDialogs::SelectFile("Select an asset", filter);
			if (std::filesystem::exists(path)) // Selected a valid file
				return path;
		}

		return ""; // No change
	}

	ByteInput::ByteInput(const std::string& label, char& value)
		: Input(value)
	{
		Internal::SetupInput(label);
		m_changed = ImGui::InputScalar(("##" + label).c_str(), ImGuiDataType_S8, &m_value);
		CacheHovered();
	}

	IntInput::IntInput(const std::string& label, int& value)
		: Input(value)
	{
		Internal::SetupInput(label);
		m_changed = ImGui::InputInt(("##" + label).c_str(), &m_value);
		CacheHovered();
	}

	FloatInput::FloatInput(const std::string& label, float& value)
		: Input(value)
	{
		Internal::SetupInput(label);
		m_changed = ImGui::InputFloat(("##" + label).c_str(), &m_value);
		CacheHovered();
	}

	CheckBox::CheckBox(const std::string& label, bool& value)
		: Input(value)
	{
		Internal::SetupInput(label);
		m_changed = ImGui::Checkbox(("##" + label).c_str(), &m_value);
		CacheHovered();
	}

	Button::Button(const std::string& label)
	{
		auto& style = ImGui::GetStyle();
		const Vector2 size{ ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2.0f,
		              ImGui::GetFrameHeight()};
		Anchor::SetCursorPos(size);
		m_clicked = ImGui::Button(label.c_str());
		CacheHovered();
	}

	bool Button::IsClicked(RexEngine::MouseButton mouseButton, MouseAction action) const
	{
		if (mouseButton == MouseButton::Left && action == MouseAction::Clicked)
			return m_clicked;
		else
			return Clickable::IsClicked(mouseButton);
	}

	Icon::Icon(const std::string& label, const RexEngine::Texture& icon, Vector2 iconSize)
	{
		auto& style = ImGui::GetStyle();
		bool selected = false;

		const Vector2 totalSize = iconSize + 
			Vector2{0, ImGui::CalcTextSize(label.c_str(), 0, false, iconSize.x).y + style.ItemSpacing.y};

		Anchor::SetCursorPos(totalSize);
		const ImVec2 cursor = ImGui::GetCursorPos();
		ImGui::Selectable(("##selectable_" + label).c_str(), &selected, ImGuiSelectableFlags_None, Internal::VecConvert(totalSize));

		m_clicked = ImGui::IsItemClicked();
		CacheHovered(); // Hovered is on the selectable

		ImGui::SetCursorPos(cursor);
		ImGui::Image((ImTextureID)icon.GetId(), Internal::VecConvert(iconSize));

		// Centered text
		ImGui::SetCursorPosY(cursor.y + iconSize.y + style.ItemSpacing.y);
		ImGui::SetCursorPosX(cursor.x + 
			(iconSize.x - ImGui::CalcTextSize(label.c_str(), 0, false, iconSize.x).x) * 0.5f);
		ImGui::TextWrapped(label.c_str(), (float)iconSize.x);
		ImGui::Selectable(("##selectable_" + label).c_str()); // Act as the whole selectable area for other tools (like DragDrop)
	}

	bool Icon::IsClicked(RexEngine::MouseButton mouseButton, MouseAction action) const
	{
		if (mouseButton == MouseButton::Left && action == MouseAction::Clicked)
			return m_clicked;
		else
			return Clickable::IsClicked(mouseButton);
	}



	FloatSlider::FloatSlider(const std::string& label, float min, float max, float width, float& value, int precision)
		: Input(value)
	{
		auto& style = ImGui::GetStyle();
		const Vector2 size = { width + ImGui::CalcTextSize(label.c_str()).x + style.ItemSpacing.x,
						 ImGui::GetFrameHeight() };

		// Label to the left
		Anchor::SetCursorPos(size);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label.c_str());
		ImGui::SameLine();
		ImGui::SetNextItemWidth(width);
		ImGui::SliderFloat(("##" + label).c_str(), &m_value, min, max, ("%." + std::to_string(precision) + "f").c_str());
		CacheHovered();
	}


	ComboBox::ComboBox(const std::string& label, std::vector<std::string> options, int& selected)
		: Input(selected)
	{
		std::string optionsStr = "";
		for (auto& str : options)
			optionsStr += str + '\0';

		// Label to the left
		Internal::SetupInput(label);
		m_changed = ImGui::Combo(("##" + label).c_str(), &m_value, optionsStr.data(), options.size());
		CacheHovered();
	}


	TreeNodeFlags operator | (TreeNodeFlags lhs, TreeNodeFlags rhs)
	{
		using T = std::underlying_type_t <TreeNodeFlags>;
		return static_cast<TreeNodeFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
	}

	TreeNodeFlags& operator |= (TreeNodeFlags& lhs, TreeNodeFlags rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	TreeNode::TreeNode(const std::string& label, TreeNodeFlags flags) 
		: m_shouldPop(!((int)flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
	{
		// Ripped the size calculation from the imgui treenode function
		auto& style = ImGui::GetStyle();
		const ImVec2 label_size = ImGui::CalcTextSize(label.c_str());
		const bool display_frame = ((int)flags & ImGuiTreeNodeFlags_Framed) != 0;
		const ImVec2 padding = (display_frame || ((int)flags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : style.FramePadding;

		Anchor::SetCursorPos({ ImGui::GetFontSize() + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f),
			ImGui::GetFrameHeight() });
		m_open = ImGui::TreeNodeEx(label.c_str(), (int)flags);
		CacheHovered();
	}

	TreeNode::~TreeNode()
	{
		if (m_open && m_shouldPop)
			ImGui::TreePop();
	}


	Table::Table(const std::string& name, int nbCols)
	{
		m_visible = ImGui::BeginTable(name.c_str(), nbCols);
	}

	Table::~Table()
	{
		if(m_visible)
			ImGui::EndTable();
	}

	void Table::SetCellPadding(Vector2Int padding)
	{
		auto table = ImGui::GetCurrentTable();
		table->CellPaddingX = padding.x;
		table->CellPaddingY = padding.y;
	}

	void Table::NextElement()
	{
		ImGui::TableNextColumn();
	}

	void Table::PreviousElement()
	{
		auto index = ImGui::TableGetColumnIndex();
		ImGui::TableSetColumnIndex(index - 1);
	}


	Text::Text(const std::string& text)
	{
		Anchor::SetCursorPos(Internal::VecConvert(ImGui::CalcTextSize(text.c_str())));
		ImGui::Text(text.c_str());
		CacheHovered();
	}

	FramedText::FramedText(const std::string& text, bool border, RexEngine::Vector2 padding)
	{
		auto& style = ImGui::GetStyle();
		const ImVec2 size{
			(padding.x == -1 ? ImGui::GetContentRegionAvail().x : (ImGui::CalcTextSize(text.c_str()).x + style.FramePadding.x * 2 + padding.x)),
			ImGui::CalcTextSize(text.c_str()).y + style.FramePadding.y * 2 + padding.y
		};
		const float paddingX = size.x - ImGui::CalcTextSize(text.c_str()).x;

		Anchor::SetCursorPos(Internal::VecConvert(size));
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImRect bb(pos, { pos.x + size.x, pos.y + size.y });
		const ImU32 col = ImGui::GetColorU32(ImGuiCol_FrameBg);

		ImGui::RenderFrame(bb.Min, bb.Max, col, border, style.FrameRounding);
		CacheHovered();
		ImGui::AlignTextToFramePadding();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (paddingX / 2));
		ImGui::Text(text.c_str());
	}


	Menu::Menu(const std::string& label, bool enabled)
	{
		m_open = ImGui::BeginMenu(label.c_str(), enabled);
	}

	Menu::~Menu()
	{
		if (m_open)
			ImGui::EndMenu();
	}

	MenuItem::MenuItem(const std::string& name, bool enabled)
		: m_clicked(false)
	{
		ImGui::MenuItem(name.c_str(), NULL, &m_clicked, enabled);
		CacheHovered();
	}

	bool MenuItem::IsClicked(RexEngine::MouseButton mouseButton, MouseAction action) const
	{
		if (mouseButton == MouseButton::Left && action == MouseAction::Clicked)
			return m_clicked;
		else
			Clickable::IsClicked(mouseButton);
	}



	Popup::Popup(const std::string& name)
	{
		m_open = ImGui::BeginPopup(name.c_str());
	}

	Popup::~Popup()
	{
		if(m_open)
			ImGui::EndPopup();
	}

	void Popup::Open(const std::string& name)
	{
		ImGui::OpenPopup(name.c_str());
	}

	ContextMenu::ContextMenu(const std::string& name)
	{
		m_open = ImGui::BeginPopupContextWindow(name.c_str());
	}
	ContextMenu::~ContextMenu()
	{
		if (m_open)
			ImGui::EndPopup();
	}
}
