#include <REPch.h>
#include "MonoApi.h"

#include "ScriptComponent.h"

#include <scene/Scene.h>
#include <inputs/Inputs.h>
#include <inputs/Keyboard.h>
#include <inputs/Mouse.h>

namespace RexEngine
{
	std::shared_ptr<ScriptType> MonoApi::GetScriptType(const Mono::Class& class_)
	{
		for (auto& type : s_scriptTypes)
		{
			if (type->GetClass() == class_)
				return type;
		}

		return nullptr;
	}

	void MonoApi::MonoStart()
	{
		RegisterLog();
		RegisterGuid();
		RegisterScene();
		RegisterInputs();
		s_apiAssembly = Mono::Assembly::Load("mono/CSharpApi.dll");

		// Cache the C# type for each c++ component
		for (auto& factory : ComponentFactories::GetFactories())
		{
			auto class_ = s_apiAssembly->GetClass("RexEngine", factory->GetName());
			if(class_.has_value())
				s_componentClasses.insert({factory->GetName(), class_.value()});
		}
	}

	void MonoApi::OnUpdate()
	{
		auto scene = Scene::CurrentScene();
		if (!scene)
			return;

		auto scriptComponents = scene->GetComponents<ScriptComponent>();
		for (auto&& [entity, scriptComponent] : scriptComponents)
		{
			for (auto& script : scriptComponent.Scripts())
			{
				script.CallOnUpdate();
			}
		}
	}

	void MonoApi::OnAddClass([[maybe_unused]]const Mono::Assembly& assembly, Mono::Class class_)
	{
		const Mono::Assembly* apiAssembly = s_apiAssembly.get();
		if (!s_apiAssembly)
			apiAssembly = &assembly;

		auto scriptComponentClass = apiAssembly->GetClass("RexEngine", "ScriptComponent").value();
		if (class_.IsSubClassOf(scriptComponentClass))
		{
			s_scriptTypes.push_back(std::make_shared<ScriptType>(class_));
		}
	}

	void MonoApi::OnRemoveClass([[maybe_unused]]const Mono::Assembly& assembly, Mono::Class class_)
	{
		auto scene = Scene::CurrentScene();
		if (!scene)
			return;

		for (auto&& [e, c] : scene->GetComponents<ScriptComponent>())
		{
			c.RemoveScriptType(class_);
		}

		s_scriptTypes.erase(std::remove_if(s_scriptTypes.begin(), s_scriptTypes.end(), [&class_](std::shared_ptr<ScriptType> type) {return type->GetClass() == class_; }), s_scriptTypes.end());
	}

	void MonoApi::OnSceneStart(Asset<Scene> scene)
	{
		// Call OnStart() on all the scripts
		for (auto& [e, c] : scene->GetComponents<ScriptComponent>())
		{
			for (auto& s : c.Scripts())
			{
				s.CallOnStart();
			}
		}
	}

	void MonoApi::OnSceneStop(Asset<Scene> scene)
	{
		// Call OnDestroy() on all the scripts
		for (auto& [e, c] : scene->GetComponents<ScriptComponent>())
		{
			for (auto& s : c.Scripts())
			{
				s.CallOnDestroy();
			}
		}
	}

	template<Log::LogType LogType>
	static void LogMessage(MonoString* message, int line, MonoString* funcName, MonoString* fileName)
	{
		Log::DispatchLog(LogType, line, Mono::GetString(funcName), Mono::GetString(fileName), Mono::GetString(message));
	}

	void MonoApi::RegisterLog()
	{
		Mono::RegisterCall("RexEngine.Log::Info", LogMessage<Log::LogType::Info>);
		Mono::RegisterCall("RexEngine.Log::Warning", LogMessage<Log::LogType::Warning>);
		Mono::RegisterCall("RexEngine.Log::Error", LogMessage<Log::LogType::Error>);
		Mono::RegisterCall("RexEngine.Log::Debug", LogMessage<Log::LogType::Debug>);
	}

	static MonoString* GuidToString(Guid guid)
	{
		return Mono::MakeString(guid.ToString());
	}

	void MonoApi::RegisterGuid()
	{
		Mono::RegisterCall("RexEngine.GUID::Generate", Guid::Generate);
		Mono::RegisterCall("RexEngine.GUID::GuidToString", GuidToString);
	}

	void MonoApi::RegisterScene()
	{
		Mono::RegisterCall("RexEngine.Entity::IsEntityAlive", [](Guid guid) -> bool { return Entity(guid).operator bool(); });
		Mono::RegisterCall("RexEngine.Entity::GetEntityName", [](Guid guid) -> MonoString* { return Mono::MakeString(Entity(guid).Name()); });
		Mono::RegisterCall("RexEngine.Entity::EntityGetComponent", [](Guid guid, MonoString* name) -> MonoObject* {
				const Entity e(guid);
				const std::string nameStr = Mono::GetString(name);
			
				// C# Types
				if (e.HasComponent<ScriptComponent>())
				{
					auto c = e.GetComponent<ScriptComponent>().GetScript(nameStr);
					if (c.has_value())
						return c.value().GetPtr();
				}

				// C++ Types
				auto factory = ComponentFactories::GetFactory(nameStr);
				if (factory && factory->HasComponent(e) && s_componentClasses.contains(nameStr))
				{
					auto obj = Mono::Object::Create(s_componentClasses.at(nameStr));
					if(obj.has_value())
						return obj.value().GetPtr();
				}

				return nullptr;
			});

		Mono::RegisterCall("RexEngine.Entity::EntityAddComponent", [](Guid guid, MonoString* name) -> MonoObject* {
				Entity e(guid);
				const std::string nameStr = Mono::GetString(name);

				// C++ Types
				auto factory = ComponentFactories::GetFactory(nameStr);
				if (factory && s_componentClasses.contains(nameStr))
				{
					if (factory->HasComponent(e))
						return nullptr;

					factory->AddComponent(e);
					auto obj = Mono::Object::Create(s_componentClasses.at(nameStr));
					if (obj.has_value())
						return obj.value().GetPtr();
				}

				// C# Types
				if (e.HasComponent<ScriptComponent>())
				{
					auto class_ = Mono::Assembly::FindClass("RexEngine", nameStr);
					if(class_.has_value())
					{ 
						auto scriptType = MonoApi::GetScriptType(class_.value());
						if (scriptType)
						{
							return e.GetComponent<ScriptComponent>().AddScript(scriptType).GetPtr();
						}
					}
				}

				return nullptr;
			});

		Mono::RegisterCall("RexEngine.Entity::EntityRemoveComponent", [](Guid guid, MonoString* name) -> bool {
				Entity e(guid);
				const std::string nameStr = Mono::GetString(name);

				// C++ Types
				auto factory = ComponentFactories::GetFactory(nameStr);
				if (factory)
				{
					return factory->RemoveComponent(e);
				}

				// C# Types
				if (e.HasComponent<ScriptComponent>())
				{
					auto class_ = Mono::Assembly::FindClass("RexEngine", nameStr);
					if (class_.has_value())
					{
						return e.GetComponent<ScriptComponent>().RemoveScriptType(class_.value()) > 0;
					}
				}

				return false;
			});
	}

	void MonoApi::RegisterInputs()
	{
		Mono::RegisterCall("RexEngine.Inputs::AddActionInternal", [](MonoString* namePtr) { Inputs::AddAction(Mono::GetString(namePtr)); });
		
		Mono::RegisterCall("RexEngine.Action::ActionIsDown", [](MonoString* namePtr) -> bool { return Inputs::GetAction(Mono::GetString(namePtr)).IsDown(); });
		Mono::RegisterCall("RexEngine.Action::ActionIsJustDown", [](MonoString* namePtr) -> bool { return Inputs::GetAction(Mono::GetString(namePtr)).IsJustDown(); });
		Mono::RegisterCall("RexEngine.Action::ActionIsJustUp", [](MonoString* namePtr) -> bool { return Inputs::GetAction(Mono::GetString(namePtr)).IsJustUp(); });
		Mono::RegisterCall("RexEngine.Action::ActionValue", [](MonoString* namePtr) -> float { return Inputs::GetAction(Mono::GetString(namePtr)).GetValue(); });
		Mono::RegisterCall("RexEngine.Action::ActionAddKeyboardBinding", [](MonoString* namePtr, int positive, int negative) { Inputs::GetAction(Mono::GetString(namePtr)).AddBinding<KeyboardInput>((KeyCode)positive, (KeyCode)negative); });
		Mono::RegisterCall("RexEngine.Action::ActionAddMouseButtonBinding", [](MonoString* namePtr, int positive, int negative) { Inputs::GetAction(Mono::GetString(namePtr)).AddBinding<MouseButtonInput>((MouseButton)positive, (MouseButton)negative); });
		Mono::RegisterCall("RexEngine.Action::ActionAddMouseBinding", [](MonoString* namePtr, int type) { Inputs::GetAction(Mono::GetString(namePtr)).AddBinding<MouseInput>((MouseInputType)type); });
		
		Mono::RegisterCall("RexEngine.Cursor::SetCursorMode", [](CursorMode mode) { Cursor::SetCursorMode(mode); });
	}
}

