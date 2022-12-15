#pragma once

#include <string>
#include <any>

namespace RexEditor::UI::Internal
{
	bool DragSource(const std::string& dragLabel, const std::string& toolTip);
	bool DropTarget(const std::string& dragLabel);
}

namespace RexEditor::UI
{
	class DragDrop
	{
	public:
		template<typename T>
		inline static void Source(const std::string& dragLabel, const T& payload, const std::string& toolTip = "")
		{
			if (Internal::DragSource(dragLabel, toolTip))
				s_payload = payload;
		}

		template<typename T>
		inline static std::shared_ptr<T> Target(const std::string& dragLabel)
		{
			if (Internal::DropTarget(dragLabel))
				return std::make_shared<T>(std::any_cast<T>(s_payload));

			return std::shared_ptr<T>(nullptr);
		}


	private:
		inline static std::any s_payload;
	};
}