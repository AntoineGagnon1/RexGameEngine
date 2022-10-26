#pragma once

#include "Input.h"

#include <memory>
#include <vector>

namespace RexEngine
{
	class Action
	{
	public:

		template<IInput T, typename ...Args>
		void AddBinding(Args&& ...args)
		{
			auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
			m_bindings.push_back(std::move(ptr));
		}

		// Returns true if one of the binding is down
		bool IsDown() const
		{
			for (auto&& binding : m_bindings)
				if (binding->IsDown())
					return true;

			return false;
		}
		// Returns true if one of the binding is justDown
		bool IsJustDown() const
		{
			for (auto&& binding : m_bindings)
				if (binding->IsJustDown())
					return true;

			return false;
		}

		// Returns true if one of the binding is justUp
		bool IsJustUp() const
		{
			for (auto&& binding : m_bindings)
				if (binding->IsJustUp())
					return true;

			return false;
		}

		// Returns the biggest absolute value in the bindings
		float GetValue() const
		{
			float maxValueAbs = 0.0f;
			float maxValue = 0.0f;
			
			for (auto&& binding : m_bindings)
			{
				float val = binding->GetValue();
				if (abs(val) > maxValueAbs)
				{
					maxValue = val;
					maxValueAbs = abs(val);
				}
			}
			
			return maxValue;
		}

		// Internal use
		void PollInputs()
		{
			for (auto&& binding : m_bindings)
				binding->PollInputs();
		}

	private:
		std::vector<std::unique_ptr<Input>> m_bindings;
	};
}