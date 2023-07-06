#pragma once

#include <unordered_map>
#include <typeIndex>
#include <functional>
#include <memory>

#include "Entity.h"

namespace RexEngine
{
	#define RE_REGISTER_COMPONENT(TYPE, NAME) namespace Components{ \
		inline CreateFactory<TYPE> Component##TYPE##Factory = CreateFactory<TYPE>(NAME); \
	}

	class ComponentFactory
	{
	public:
		template<typename T>
		using f = std::function<T>;

		ComponentFactory(std::type_index type, const std::string& name, f<void(Entity&)> addComponent, f<bool(const Entity&)> hasComponent, f<bool(Entity&)> removeComponent, f<void(Entity&, JsonDeserializer&)> loadJson, f<void(const Entity&, JsonSerializer&)> saveJson)
			: m_type(type), m_name(name), m_addComponent(addComponent), m_hasComponent(hasComponent), m_removeComponent(removeComponent), m_loadFromJson(loadJson), m_saveToJson(saveJson)
		{ }

		auto GetType() const { return m_type; }
		const auto& GetName() const { return m_name; }

		bool HasComponent(const Entity& e) const { return m_hasComponent(e); }
		bool RemoveComponent(Entity& e) const { return m_removeComponent(e); }
		void AddComponent(Entity& e) const { m_addComponent(e); }

		void FromJson(Entity& e, JsonDeserializer& archive) const { m_loadFromJson(e, archive); }
		void ToJson(const Entity& e, JsonSerializer& archive) const { m_saveToJson(e, archive); }

	private:
		std::type_index m_type;
		std::string m_name;

		f<void(Entity&)> m_addComponent;
		f<bool(const Entity&)> m_hasComponent;
		f<bool(Entity&)> m_removeComponent;
		f<void(Entity&, JsonDeserializer&)> m_loadFromJson;
		f<void(const Entity&, JsonSerializer&)> m_saveToJson;
	};

	class ComponentFactories
	{
	public:
		// Will return nullptr if the type was not found
		// the ptr might change if new factories are added, DO NOT cache it
		inline static const ComponentFactory* GetFactory(std::type_index type)
		{
			for (auto& f : GetFactories())
			{
				if (f->GetType() == type)
					return f.get();
			}

			return nullptr;
		}

		// Will return nullptr if the name was not found
		inline static const ComponentFactory* GetFactory(const std::string& name)
		{
			for (auto& f : GetFactories())
			{
				if (f->GetName() == name)
					return f.get();
			}

			return nullptr;
		}

		// vector because the count will be small, and search is by type and name
		inline static std::vector<std::unique_ptr<ComponentFactory>>& GetFactories()
		{
			static std::vector<std::unique_ptr<ComponentFactory>> factories;
			return factories;
		}
	};

	template<typename T>
	concept HasRemoveComponent = requires (Entity e) { {e.RemoveComponent<T>()}; };

	template<typename T>
	concept HasAddComponent = requires (Entity e) { {e.AddComponent<T>()}; };

	template<typename T>
	struct CreateFactory
	{
		CreateFactory(std::string&& name)
		{
			ComponentFactories::GetFactories().emplace_back(std::make_unique<ComponentFactory>(typeid(T), std::forward<std::string>(name), 
				[]([[maybe_unused]]Entity& e) { // AddComponent
					if constexpr (HasAddComponent<T>)
						e.AddComponent<T>();
				},
				[](const Entity& e) { // HasComponent
					return e.HasComponent<T>();
				},
				[]([[maybe_unused]] Entity& e) { // RemoveComponent
					if constexpr (HasRemoveComponent<T>)
						return e.RemoveComponent<T>();
					else
						return false;
				},
				[name](Entity& e, JsonDeserializer& archive) { // SaveJson
					if constexpr (HasAddComponent<T>)
					{
						if (!e.HasComponent<T>())
							e.AddComponent<T>();
					}

					archive(CUSTOM_NAME(e.GetComponent<T>(), name));
				},
				[name](const Entity& e, JsonSerializer& archive) { // ToJson
					if (e.HasComponent<T>())
						archive(CUSTOM_NAME(e.GetComponent<T>(), name));
				}
			));
		}

		~CreateFactory()
		{
			auto& fs = ComponentFactories::GetFactories();
			for (int i = 0; i < fs.size(); i++)
			{
				if (fs[i]->GetType() == typeid(T))
				{
					fs.erase(fs.begin() + i);
					return;
				}
			}
		}
	};
}