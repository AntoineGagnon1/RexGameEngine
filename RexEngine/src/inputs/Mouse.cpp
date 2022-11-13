#include <REPch.h>
#include "Mouse.h"

#include "core/Libs.h"

#include "window/Window.h"

namespace
{
	int MouseButtonToGLFW[] = {
		GLFW_MOUSE_BUTTON_RIGHT,
		GLFW_MOUSE_BUTTON_LEFT,
		GLFW_MOUSE_BUTTON_MIDDLE,
		GLFW_MOUSE_BUTTON_4,
		GLFW_MOUSE_BUTTON_5,
		-1 // None
	};
}

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
		using T = decltype(m_value);

		if (Window::ActiveWindow() == nullptr)
			return;

		double x, y;
		glfwGetCursorPos(Window::ActiveWindow()->WindowHandle(), &x, &y);

		switch (m_type)
		{
		case RexEngine::MouseInputType::X:
			m_value = (T)x / (T)Window::ActiveWindow()->GetSize().x;
			break;

		case RexEngine::MouseInputType::Y:
			m_value = (T)y / (T)Window::ActiveWindow()->GetSize().y;
			break;


		case RexEngine::MouseInputType::DeltaX:
			m_value = ((T)x - m_lastValue) / (T)Window::ActiveWindow()->GetSize().x;
			m_lastValue = (T)x;
			break;

		case RexEngine::MouseInputType::DeltaY:
			m_value = ((T)y - m_lastValue) / (T)Window::ActiveWindow()->GetSize().y;
			m_lastValue = (T)y;
			break;


		case RexEngine::MouseInputType::RawX:
			m_value = (T)x;
			break;

		case RexEngine::MouseInputType::RawY:
			m_value = (T)y;
			break;
		}
	}

	MouseButtonInput::MouseButtonInput(MouseButton positive, MouseButton negative)
		: m_positive(positive), m_negative(negative)
	{ }

	void MouseButtonInput::PollInputs()
	{
		if (Window::ActiveWindow() == nullptr)
			return;

		bool positive = m_positive == MouseButton::None ? false : (glfwGetMouseButton(Window::ActiveWindow()->WindowHandle(), MouseButtonToGLFW[(int)m_positive]) == GLFW_PRESS);
		bool negative = m_negative == MouseButton::None ? false : (glfwGetMouseButton(Window::ActiveWindow()->WindowHandle(), MouseButtonToGLFW[(int)m_negative]) == GLFW_PRESS);

		bool newDown = positive; // Only the positive key is considered for down/justDown/justUp

		m_justDown = newDown && !m_down;
		m_justUp = !newDown && m_down;

		m_down = newDown;

		m_value = (positive ? 1.0f : 0.0f) + (negative ? -1.0f : 0.0f);
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