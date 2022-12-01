#pragma once
#include <array>

#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/details/util.hpp>
#include <cereal/types/array_better.hpp> // DO NOT include cereal/types/array, a custom serializer is used to generate json arrays

#include <entt/entt.hpp>


namespace RexEngine
{
	using JsonSerializer = cereal::JSONOutputArchive;
	using JsonDeserializer = cereal::JSONInputArchive;

	#define KEEP_NAME(__var__) CEREAL_NVP(__var__)
	#define CUSTOM_NAME(__var__, __name__) cereal::make_nvp(__name__, __var__)

	template<typename Serializer> // ex : JsonSerializer
	class OutputArchive
	{
	public:

		OutputArchive(Serializer& output) : m_output(output) {}

		// count is the number of entity that will be stored
		void operator()(std::underlying_type_t<entt::entity> count)
		{
			m_output(CUSTOM_NAME(count, "Count"));
		}

		void operator()(entt::entity e)
		{
			m_output(CUSTOM_NAME(e, "Entity"));
		}

		template<typename T>
		void operator()(entt::entity e, const T& c)
		{
			m_output(CUSTOM_NAME(e, "Owner"), CUSTOM_NAME(c, typeid(c).name()));
		}

	private:
		Serializer& m_output;
	};

	template<typename Deserializer> // ex : JsonDeserializer
	class InputArchive
	{
	public:

		InputArchive(Deserializer& input) : m_input(input) {}

		// count is the number of entity that will be stored
		void operator()(std::underlying_type_t<entt::entity>& count) const
		{
			m_input(CUSTOM_NAME(count, "Count"));
		}

		void operator()(entt::entity& e) const
		{
			m_input(CUSTOM_NAME(e, "Entity"));
		}

		template<typename T>
		void operator()(entt::entity& e, T& c) const
		{
			m_input(CUSTOM_NAME(e, "Owner"), CUSTOM_NAME(c, typeid(c).name()));
		}

	private:
		Deserializer& m_input;
	};
}