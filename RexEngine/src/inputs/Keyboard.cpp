#include <REPch.h>
#include "Keyboard.h"

#include "core/Libs.h"

#include "window/Window.h"

namespace
{
	int KeyCodeToGLFW[] = {
		-1,
		32,
		39,
		44,
		45,
		46,
		47,
		48,
		49,
		50,
		51,
		52,
		53,
		54,
		55,
		56,
		57,
		59,
		61,
		65,
		66,
		67,
		68,
		69,
		70,
		71,
		72,
		73,
		74,
		75,
		76,
		77,
		78,
		79,
		80,
		81,
		82,
		83,
		84,
		85,
		86,
		87,
		88,
		89,
		90,
		91,
		92,
		93,
		96,
		256,
		257,
		258,
		259,
		260,
		261,
		262,
		263,
		264,
		265,
		266,
		267,
		268,
		269,
		280,
		281,
		282,
		283,
		284,
		290,
		291,
		292,
		293,
		294,
		295,
		296,
		297,
		298,
		299,
		300,
		301,
		302,
		303,
		304,
		305,
		306,
		307,
		308,
		309,
		310,
		311,
		312,
		313,
		314,
		320,
		321,
		322,
		323,
		324,
		325,
		326,
		327,
		328,
		329,
		330,
		331,
		332,
		333,
		334,
		335,
		336,
		340,
		341,
		342,
		343,
		344,
		345,
		346,
		347,
		348
	};
}

namespace RexEngine
{
	KeyboardInput::KeyboardInput(KeyCode positive, KeyCode negative)
		:m_positive(positive), m_negative(negative)
	{ }

	void KeyboardInput::PollInputs()
	{
		if (Window::ActiveWindow() == nullptr)
			return;

		bool positive = glfwGetKey(Window::ActiveWindow()->WindowHandle(), KeyCodeToGLFW[(int)m_positive]) == GLFW_PRESS;
		bool negative = glfwGetKey(Window::ActiveWindow()->WindowHandle(), KeyCodeToGLFW[(int)m_negative]) == GLFW_PRESS;

		bool newDown = positive; // Only the positive key is considered for down/justDown/justUp

		m_justDown = newDown && !m_down;
		m_justUp = !newDown && m_down;

		m_down = newDown;

		m_value = (positive ? 1.0f : 0.0f) + (negative ? -1.0f : 0.0f);
	}
}