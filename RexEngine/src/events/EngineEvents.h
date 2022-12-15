#pragma once

#include "Event.h"

#define RE_DECL_EVENT(name, ...) inline static auto& name() { static RexEngine::Event<__VA_ARGS__> e; return e; }

namespace RexEngine
{
	class EngineEvents
	{
	public:
		
		RE_DECL_EVENT(OnEngineStart) // Should only be called once, when the engine is started

		RE_DECL_EVENT(OnPreUpdate) // Runs before the update, poll events, calculate the delta time, ...
		RE_DECL_EVENT(OnUpdate) // Update event, update the ECS, ScriptEngine, ...

		RE_DECL_EVENT(OnEngineStop) // Should only be called once, before the app closes

	private:
	};
}