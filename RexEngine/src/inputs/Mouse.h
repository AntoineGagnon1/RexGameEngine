#pragma once

#include "Input.h"

namespace RexEngine
{
	// Raw is in pixels, others are from -1 to 1
	enum class MouseInputType { X, Y, DeltaX, DeltaY, RawX, RawY };

	class MouseInput final : public Input
	{
	public:
		MouseInput(MouseInputType type);

		void PollInputs() override;

	private:
		MouseInputType m_type;
		decltype(m_value) m_lastValue;
	};

	enum class MouseButton { Right, Left, Middle, Back, Front, None };

	class MouseButtonInput : public Input
	{
	public:
		MouseButtonInput(MouseButton positive, MouseButton negative = MouseButton::None);

		void PollInputs() override;

	private:
		MouseButton m_positive;
		MouseButton m_negative;
	};


	enum class CursorMode { Free, Locked };

	class Cursor
	{
	public:

		static void SetCursorMode(CursorMode mode);
	};
}