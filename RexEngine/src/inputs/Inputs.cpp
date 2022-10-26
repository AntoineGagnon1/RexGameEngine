#include <REPch.h>
#include "Inputs.h"

#include "core/Libs.h"

namespace RexEngine
{
	Action& Inputs::AddAction(const std::string& name)
	{
		RE_ASSERT(!m_actions.contains(name), "Action {} already exists !", name);
		m_actions.insert({ name, Action() });
		return m_actions[name];
	}

	Action& Inputs::GetAction(const std::string& name)
	{
		RE_ASSERT(m_actions.contains(name), "Action {} does not exist !", name);
		return m_actions[name];
	}

	void Inputs::PollInputs()
	{
		glfwPollEvents();

		for (auto&& action : m_actions)
			action.second.PollInputs();
	}
}