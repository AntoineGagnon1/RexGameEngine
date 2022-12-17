#pragma once

#include <string>
#include <memory>
#include <functional>

#include <RexEngine.h>

#include "Panel.h"
#include "ui/MenuBar.h"
#include "core/EditorEvents.h"

namespace RexEditor
{
	class PanelManager
	{
	public:

		// Leave menuPath empty to not create a menu button
		template<RexEngine::IsDerivedFrom<Panel> T>
		inline static void RegisterPanel(const std::string& menuPath = "")
		{
			s_panels.push_back(std::make_unique<T>());
			Panel* panel = s_panels.back().get();

			if(!menuPath.empty())
				UI::MenuBar::RegisterMenuFunction("Windows/" + menuPath, [panel] { panel->Show(); });
		}

		inline static void RemovePanel(Panel* ptr)
		{
			for(int i = 0; i < s_panels.size(); i++)	
			{
				if (s_panels[i].get() == ptr)
				{
					s_panels.erase(s_panels.begin() + i);
					return;
				}
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

		template<RexEngine::IsDerivedFrom<Panel> T>
		inline static T* GetPanel()
		{
			for (auto& p : s_panels)
			{
				T* cast = dynamic_cast<T*>(p.get());
				if (cast != nullptr)
					return cast;
			}
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

		inline static void RenderPanels(float deltaTime)
		{
			for (auto& p : s_panels)
			{
				p->Render(deltaTime);
			}
		}

		RE_STATIC_CONSTRUCTOR({
			EditorEvents::OnEditorStop().Register<&PanelManager::OnStop>();
			EditorEvents::OnUI().Register<&PanelManager::RenderPanels>();
		});

	private:

		inline static std::vector<std::unique_ptr<Panel>> s_panels;
	};
}