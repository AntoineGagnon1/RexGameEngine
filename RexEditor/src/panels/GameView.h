#pragma once

#include <RexEngine.h>

#include "Panel.h"
#include "ui/UIElements.h"

namespace RexEditor
{
	class GameViewPanel : public Panel
	{
	public:
		GameViewPanel() : Panel("Game View")
		{ }

	protected:
		virtual void OnGui(float deltaTime) override
		{
			
		}
	};
}