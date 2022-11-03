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

	enum class CursorMode { Free, Locked };

	class Cursor
	{
	public:

		static void SetCursorMode(CursorMode mode);
	};
}