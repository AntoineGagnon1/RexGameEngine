#pragma once

#include <string>
#include <RexEngine.h>

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

		RexEngine::Vector2Int PanelSize() const { return m_size; }
		bool IsFocused() const { return m_focused; }
		bool IsHovered() const;

		const std::string& Title() const { return m_title; }
		bool IsVisible() const { return m_open; }

	protected:
		virtual void OnGui(float deltaTime) = 0;
		virtual void OnResize(RexEngine::Vector2Int oldSize, RexEngine::Vector2Int newSize) {};
		virtual void OnFocusEnter() {};
		virtual void OnFocusLeave() {};

		// True by default
		bool CanDock() const { return m_canDock; }
		void CanDock(bool canDock) { m_canDock = canDock; }

	private:
		std::string m_title;
		RexEngine::Vector2Int m_size;
		bool m_open;
		bool m_focused;
		bool m_canDock;
	};
}