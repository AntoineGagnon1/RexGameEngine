#pragma once

#include <string>
#include <RexEngine.h>

#include "ui/UI.h"
#include "ui/UIElements.h"

namespace RexEditor
{
	class Panel
	{
	public:
		Panel(const std::string& title);
		virtual ~Panel() { }

		void Render(float deltaTime);

		void Hide() { Show(false); }
		// Set show to false to hide
		void Show(bool show = true);

		RexEngine::Vector2 PanelSize() const { return m_size; }
		bool IsFocused() const { return m_focused; }
		void SetFocused() { UI::Window::SetFocused(m_title); }
		bool IsHovered() const;

		const std::string& Title() const { return m_title; }
		bool IsVisible() const { return m_open; }

		bool IsUnsaved() const { return m_unsaved; }
		void SetUnsaved(bool unsaved) { m_unsaved = unsaved; }

	protected:
		virtual void OnGui(float deltaTime) = 0;
		virtual void OnResize([[maybe_unused]]RexEngine::Vector2 oldSize, [[maybe_unused]]RexEngine::Vector2 newSize) {};
		virtual void OnFocusEnter() {};
		virtual void OnFocusLeave() {};

		// True by default
		bool CanDock() const { return m_canDock; }
		void CanDock(bool canDock) { m_canDock = canDock; }

		UI::Window* Window() { return m_window; }

	private:
		std::string m_title;
		RexEngine::Vector2 m_size;
		bool m_open;
		bool m_focused;
		bool m_hovered;
		bool m_canDock;
		bool m_unsaved; // little dot next to the name

		UI::Window* m_window;
	};
}