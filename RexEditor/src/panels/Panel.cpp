#include "REDPch.h"
#include "Panel.h"

#include "PanelManager.h"

namespace RexEditor
{
	Panel::Panel(const std::string& title)
		: m_title(title), m_open(true), m_hovered(false), m_canDock(true)
	{ }

	void Panel::Render(float deltaTime)
	{
		if (!m_open)
			return; // panel is hidden

		UI::WindowSetting settings = UI::WindowSetting::NoCollapse;	
		settings |= m_canDock == false ? UI::WindowSetting::NoDocking : UI::WindowSetting::None;
		settings |= m_unsaved == true ? UI::WindowSetting::UnsavedDocument : UI::WindowSetting::None;
		
		UI::Window w(m_title.c_str(), &m_open, settings);
		// Cache the window
		m_window = &w;

		if (w.IsVisible())
		{
			// Handle resize
			auto newSize = w.Size();
			if (m_size != newSize)
			{
				auto oldSize = m_size;
				m_size = newSize;
				OnResize(oldSize, m_size);
			}

			// Handle focus
			auto newFocus = w.IsFocused();
			if (m_focused != newFocus)
			{
				m_focused = newFocus;
				if (m_focused)
					OnFocusEnter();
				else
					OnFocusLeave();
			}

			// Cache hovered
			m_hovered = w.IsHovered();
			OnGui(deltaTime);
		}

		m_window = nullptr; // The window object will be deleted
	}

	void Panel::Show(bool show)
	{
		m_open = show;
	}

	bool Panel::IsHovered() const
	{ 
		return m_hovered;
	}
}