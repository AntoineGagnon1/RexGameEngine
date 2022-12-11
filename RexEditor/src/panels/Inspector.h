#pragma once

#include <RexEngine.h>

#include "Panel.h"
#include "../core/EditorEvents.h"

namespace RexEditor
{
	class InspectorPanel : public Panel
	{
	public:
		InspectorPanel() : Panel("Inspector")
		{ }

		using InspectGuiCallback = std::function<void(float deltaTime)>;

		inline static void InspectElement(InspectGuiCallback callback)
		{
			s_currentElement = callback;
		}

	protected:
		virtual void OnGui(float deltaTime) override
		{
			if (s_currentElement)
				s_currentElement(deltaTime);
		}

	private:
		inline static InspectGuiCallback s_currentElement;

	};
}