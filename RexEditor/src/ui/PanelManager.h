#pragma once

#include <string>
#include <memory>
#include <functional>

#include <RexEngine.h>

#include "Panel.h"
#include "MenuBar.h"
#include "EditorEvents.h"

namespace RexEditor
{
	class PanelManager
	{
	public:

		template<RexEngine::IsDerivedFrom<Panel> T>
		inline static void RegisterPanel(const std::string& menuPath)
		{
			s_panels.push_back(std::make_unique<T>());
			auto& panel = s_panels.back();
			MenuBar::RegisterMenuFunction("Windows/" + menuPath, [&panel] { panel->Show(); });
		}

		inline static void RenderPanels(float deltaTime)
		{
			for (auto& p : s_panels)
			{
				p->Render(deltaTime);
			}
		}

		// Will return nullptr if no panel with the title where found
		inline static Panel* GetPanel(const std::string& title)
		{
			for(auto& p : s_panels)
			{
				if (p->Title() == title)
					return p.get();
			}

			return nullptr;
		}

		// Will return nullptr if the index is out if range
		inline static Panel* GetPanel(size_t index)
		{
			if (index < 0 || index >= PanelCount())
				return nullptr;
			return s_panels[index].get();
		}

		inline static size_t PanelCount() { return s_panels.size(); }

	private:

		inline static void OnStop()
		{
			s_panels.clear(); // Important, will delete the unique_ptrs
		}

		RE_STATIC_CONSTRUCTOR({
			EditorEvents::OnEditorStop().Register<&PanelManager::OnStop>();
		});

	private:

		inline static std::vector<std::unique_ptr<Panel>> s_panels;
	};
}