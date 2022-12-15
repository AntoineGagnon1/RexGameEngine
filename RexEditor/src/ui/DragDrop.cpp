#include "REDPch.h"
#include "DragDrop.h"

#include <imgui/imgui.h>

namespace RexEditor::UI
{

	bool Internal::DragSource(const std::string& dragLabel, const std::string& toolTip)
	{
		ImGuiDragDropFlags flags = ImGuiDragDropFlags_SourceAllowNullID;
		if (toolTip.empty())
			flags |= ImGuiDragDropFlags_SourceNoPreviewTooltip;

		if (ImGui::BeginDragDropSource(flags))
		{
			if (!toolTip.empty())
				ImGui::Text(toolTip.c_str());

			ImGui::SetDragDropPayload(dragLabel.c_str(), nullptr, 0);
			ImGui::EndDragDropSource();
			return true;
		}

		return false;
	}

	bool Internal::DropTarget(const std::string& dragLabel)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(dragLabel.c_str()))
			{
				if (payload != nullptr)
				{
					return true;
				}
			}
			ImGui::EndDragDropTarget();
		}

		return false;
	}
}

