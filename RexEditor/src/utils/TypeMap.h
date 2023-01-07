#pragma once

#include <unordered_map>
#include <typeindex>

namespace RexEditor
{

	// A type map is a map where the key is a type
	// 
	// Items can be added via a type index using : Add(typeid(Type), SomeValue)
	// or via a template function using : Add<Type>(SomeValue)
	// 
	// Items can be retreived and deleted in the same ways
	template<typename Value_T>
	class TypeMap
	{
	public:

		auto Add(std::type_index type, Value_T&& value) { return m_map.insert({ type, std::forward<Value_T>(value) }); }

		template<typename Key_T>
		auto Add(Value_T&& value) { return Add(typeid(Key_T), std::forward<Value_T>(value)); }


		auto Get(std::type_index type) const { return m_map.at(type); }

		template<typename Key_T>
		auto Get() const { return Get(typeid(Key_T)); }

		
		auto Remove(std::type_index type) { return m_map.erase(type); }

		template<typename Key_T>
		auto Remove() { return Remove(typeid(Key_T)); }


		auto Contains(std::type_index type) const { return m_map.contains(type); }

		template<typename Key_T>
		auto Contains() const { return Contains(typeid(Key_T)); }

		auto Size() const { return m_map.size(); }
		void Clear() { m_map.clear(); }

	private:
		std::unordered_map<std::type_index, Value_T> m_map;

	};
}