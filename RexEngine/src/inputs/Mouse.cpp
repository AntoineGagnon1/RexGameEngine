#include <REPch.h>
#include "Mouse.h"

#include "core/Libs.h"

#include "window/Window.h"

namespace RexEngine
{
	MouseInput::MouseInput(MouseInputType type)
		: m_type(type), m_lastValue(0)
	{ 
		// These will always be false
		m_justDown = false;
		m_justUp = false;
		m_down = false;
	}

	void MouseInput::PollInputs()
	{
		if (Window::ActiveWindow() == nullptr)
			return;

		double x, y;
		glfwGetCursorPos(Window::ActiveWindow()->WindowHandle(), &x, &y);

		switch (m_type)
		{
		case RexEngine::MouseInputType::X:
			m_value = x / (decltype(m_value))Window::ActiveWindow()->GetSize().x;
			break;

		case RexEngine::MouseInputType::Y:
			m_value = y / (decltype(m_value))Window::ActiveWindow()->GetSize().y;
			break;


		case RexEngine::MouseInputType::DeltaX:
			m_value = (x - m_lastValue) / (decltype(m_value))Window::ActiveWindow()->GetSize().x;
			m_lastValue = x;
			break;

		case RexEngine::MouseInputType::DeltaY:
			m_value = (y - m_lastValue) / (decltype(m_value))Window::ActiveWindow()->GetSize().y;
			m_lastValue = y;
			break;


		case RexEngine::MouseInputType::RawX:
			m_value = x;
			break;

		case RexEngine::MouseInputType::RawY:
			m_value = y;
			break;
		}
	}


	void Cursor::SetCursorMode(CursorMode mode)
	{
		if (Window::ActiveWindow() == nullptr)
			return;

		switch (mode)
		{
		case RexEngine::CursorMode::Free:
			glfwSetInputMode(Window::ActiveWindow()->WindowHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		case RexEngine::CursorMode::Locked:
			glfwSetInputMode(Window::ActiveWindow()->WindowHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			break;
		}
	}
}