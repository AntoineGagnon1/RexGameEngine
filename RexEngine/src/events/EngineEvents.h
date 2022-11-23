#pragma once

#include "Event.h"

#define RE_ENGINE_EVENT(name, ...) inline static auto& name() { static Event<__VA_ARGS__> e; return e; }

namespace RexEngine
{
	class EngineEvents
	{
	public:
		
		RE_ENGINE_EVENT(OnEngineStart) // Should only be called once, when the engine is started

		RE_ENGINE_EVENT(OnPreUpdate) // Runs before the update, poll events, calculate the delta time, ...
		RE_ENGINE_EVENT(OnUpdate) // Update event, update the ECS, ScriptEngine, ...

	private:
	};
}