#pragma once
#include <filesystem>

#include <cereal/cereal.hpp>
#include <cereal/details/traits.hpp>
#include <cereal/details/util.hpp>

#include <cereal/archives/json.hpp>

#include <cereal/types/array_better.hpp> // DO NOT include cereal/types/array, a custom serializer is used to generate json arrays
#include <cereal/types/string.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/memory.hpp>

namespace RexEngine
{
	// Usage : JsonSerializer archive(ostream);
	using JsonSerializer = cereal::JSONOutputArchive;

	// Usage : JsonDeserializer archive(istream);
	using JsonDeserializer = cereal::JSONInputArchive;

	// Used to specify the json names
	// Will use the name of the variable
	#define KEEP_NAME(__var__) CEREAL_NVP(__var__)
	// Used to specify the json names
	// Will use the second argument as the name
	#define CUSTOM_NAME(__var__, __name__) cereal::make_nvp(__name__, __var__)
}

// load/save functions for std::filesystem::path
namespace std
{
	namespace filesystem
	{
		template<class Archive>
		void CEREAL_LOAD_MINIMAL_FUNCTION_NAME(const Archive&, path& out, const std::string& in)
		{
			out = in;
		}

		template<class Archive>
		std::string CEREAL_SAVE_MINIMAL_FUNCTION_NAME(const Archive& ar, const path& p)
		{
			return p.string();
		}
	}
}