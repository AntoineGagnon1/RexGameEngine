#pragma once

#include <string>
#include <map>

#include "Action.h"
#include "../core/Assert.h"

namespace RexEngine
{
	class Inputs
	{
	public:

		static Action& AddAction(const std::string& name);

		static Action& GetAction(const std::string& name);

		// Get the name of all the loaded actions
		static std::vector<std::string> GetActions();

		// Call every frame
		static void PollInputs();
		
	private:
		inline static std::map<std::string, Action> m_actions;

	};
}
