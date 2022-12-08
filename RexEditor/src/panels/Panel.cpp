#include "REDPch.h"
#include "Panel.h"

#include "ui/Gui.h"
#include "PanelManager.h"

namespace RexEditor
{
	Panel::Panel(const std::string& title)
		: m_title(title), m_open(true), m_canDock(true)
	{ }

	void Panel::Render(float deltaTime)
	{
		if (!m_open)
			return; // panel is hidden

		WindowSetting settings = WindowSetting::NoCollapse;	
		settings |= m_canDock == false ? WindowSetting::NoDocking : WindowSetting::None;

		if (Imgui::BeginWindow(m_title.c_str(), m_open, settings))
		{
			// Handle resize
			auto newSize = Imgui::GetWindowSize();
			if (m_size != newSize)
			{
				auto oldSize = m_size;
				m_size = newSize;
				OnResize(oldSize, m_size);
			}

			// Handle focus
			auto newFocus = Imgui::IsWindowFocused();
			if (m_focused != newFocus)
			{
				m_focused = newFocus;
				if (m_focused)
					OnFocusEnter();
				else
					OnFocusLeave();
			}

			OnGui(deltaTime);
		}

		Imgui::EndWindow();
	}

	void Panel::Show(bool show)
	{
		m_open = show;
	}

	bool Panel::IsHovered() const
	{ 
		return Imgui::IsWindowHovered();
	}
}