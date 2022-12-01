#pragma once

#include <cstdint>
#include <random>
#include <chrono>
#include <functional>

#include "Serialization.h"

namespace RexEngine
{
	struct Guid
	{
	private:
		uint64_t dataHigh;
		uint64_t dataLow;

		// Give access to the default constructor
		template<typename Registry>
		friend class entt::basic_snapshot_loader;

		// Use explicit generation instead : Guid::Generate() or Guid::Empty
		Guid() : Guid(0,0) {}

	public:

		// An empty Guid
		static Guid Empty;

		// Generate a new random Guid
		inline static Guid Generate()
		{
			// The low 64bits are the unix epoch in microseconds,
			// The high 64bits is a random number

			static std::random_device s_device;
			static std::mt19937_64 s_rng(s_device());
			static std::uniform_int_distribution<uint64_t> s_distribution;

			// Unix epoch in microseconds
			uint64_t low = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			uint64_t high = s_distribution(s_rng);

			return Guid(low, high);
		}

		Guid(const Guid& from) = default;

		constexpr Guid(uint64_t low, uint64_t high)
			: dataHigh(high), dataLow(low)
		{ }


		// Format this Guid to a sring
		// Format : 00000000-00000000-00000000-00000000
		std::string ToString() const
		{
			char addressChars[36];

			// High
			snprintf(addressChars, 9, "%08X", (unsigned int)((dataHigh >> 32) & 0xffffffff));
			snprintf(&addressChars[9], 9, "%08X", (unsigned int)(dataHigh & 0xffffffff));

			// Low
			snprintf(&addressChars[18], 9, "%08X", (unsigned int)((dataLow >> 32) & 0xffffffff));
			snprintf(&addressChars[27], 9, "%08X", (unsigned int)(dataLow & 0xffffffff));

			addressChars[8] = '-';
			addressChars[17] = '-';
			addressChars[26] = '-';

			return std::string(addressChars);
		}

		// Reset to Guid::Empty
		void Reset()
		{
			dataLow = Empty.dataLow;
			dataHigh = Empty.dataHigh;
		}

		size_t GetHash() const
		{
			return dataHigh * dataLow;
		}

		template<typename Archive>
		void serialize(Archive& archive)
		{
			// Save as a 2 element array
			archive(cereal::make_size_tag((cereal::size_type)2));
			archive(dataHigh, dataLow);
		}


		auto operator<=>(Guid const&) const = default;
	};

	inline Guid Guid::Empty = Guid(0,0);

}

template <>
class std::hash<RexEngine::Guid>
{
public:
	size_t operator()(const RexEngine::Guid& guid) const
	{
		return guid.GetHash();
	}
};