#pragma once
#include "MonoEngine.h"
#include "../scene/ComponentFactory.h"

namespace RexEngine
{
	struct ScriptComponent
	{
	public:

		MonoObject* AddScript(MonoClass* class_);
		void RemoveScript(MonoObject* script);
		std::span<MonoObject* const> Scripts() const { return std::span(m_scripts.begin(), m_scripts.end()); }

		template <class Archive>
		void save([[maybe_unused]]const Archive& archive) const
		{

		}

		template <class Archive>
		void load([[maybe_unused]] const Archive& archive)
		{
			
		}

	private:
		std::vector<MonoObject*> m_scripts;
	};
	RE_REGISTER_COMPONENT(ScriptComponent, "Script")


	class MonoApi
	{
	public:

		// Returns all the script types registered
		static std::vector<MonoClass*> GetScriptTypes();

	private:
		static void MonoStart();

		RE_STATIC_CONSTRUCTOR({
			MonoEngine::OnMonoStart().Register<&MonoApi::MonoStart>();
		});
	private:
		static void RegisterLog();

	private:
		inline static MonoAssembly* s_apiAssembly = nullptr;
	};
}