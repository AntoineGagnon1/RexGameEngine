#pragma once

#include <string>
#include <map>

#include "Action.h"
#include "../core/Assert.h"
#include "../events/EngineEvents.h"
#include "../utils/StaticConstructor.h"

namespace RexEngine
{
	class Inputs
	{
	public:

		static Action& AddAction(const std::string& name);

		static Action& GetAction(const std::string& name);

		// Get the name of all the loaded actions
		static std::vector<std::string> GetActions();

	private:
		static void PollInputs();

		RE_STATIC_CONSTRUCTOR({
			EngineEvents::OnPreUpdate().Register<&Inputs::PollInputs>();
		})

	private:
		inline static std::map<std::string, Action> m_actions;

	};
}
