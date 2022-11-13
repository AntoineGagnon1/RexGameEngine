#include "REDPch.h"
#include "Panel.h"

#include "Gui.h"

namespace RexEditor
{
	Panel::Panel(const std::string& title)
		: m_title(title), m_open(true)
	{ }

	void Panel::Render(float deltaTime)
	{
		if (!m_open)
			return; // panel is hidden

		if (Gui::BeginWindow(m_title.c_str(), m_open, WindowSetting::NoCollapse))
		{
			// Handle resize
			auto newSize = Gui::GetWindowSize();
			if (m_size != newSize)
			{
				auto oldSize = m_size;
				m_size = newSize;
				OnResize(oldSize, m_size);
			}

			// Handle focus
			auto newFocus = Gui::IsWindowFocused();
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

		Gui::EndWindow();
	}

	void Panel::Hide()
	{
		m_open = false;
	}

	void Panel::Show()
	{
		m_open = true;
	}

	bool Panel::IsHovered()
	{ 
		return Gui::IsWindowHovered();
	}
}