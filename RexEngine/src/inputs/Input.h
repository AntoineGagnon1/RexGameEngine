#pragma once
#include <type_traits>

namespace RexEngine
{
	// Both an axis an a button
	class Input
	{
	public:
		virtual ~Input() {}

		bool IsDown() const { return m_down; }
		bool IsJustDown() const { return m_justDown; }
		bool IsJustUp() const { return m_justUp; }

		// From -1 to 1
		float GetValue() { return m_value; }
		
		// Used internally to set down, justDown, justUp, and value
		virtual void PollInputs() = 0;

	protected:
		bool m_down = false, m_justDown = false, m_justUp = false;
		float m_value = 0.0f;
	};

	template <typename T>
	concept IInput = std::is_base_of<Input, T>::value;

	class EmptyInput final : public Input
	{
	public:
		void PollInputs() override {}
	};
}