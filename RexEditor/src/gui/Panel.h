#pragma once

#include <string>

namespace RexEditor
{
	class Panel
	{
	public:
		Panel(const std::string& title);
		virtual ~Panel() { }

		void Render(float deltaTime);

		void Hide();
		void Show();

		Vector2Int PanelSize() { return m_size; }
		bool IsFocused() { return m_focused; }
		bool IsHovered();

	protected:
		virtual void OnGui(float deltaTime) = 0;
		virtual void OnResize(Vector2Int oldSize, Vector2Int newSize) {};
		virtual void OnFocusEnter() {};
		virtual void OnFocusLeave() {};

	private:
		std::string m_title;
		Vector2Int m_size;
		bool m_open;
		bool m_focused;
	};
}