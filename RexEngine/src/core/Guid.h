#pragma once

#include <cstdint>
#include <random>
#include <chrono>

namespace RexEngine
{
	struct Guid
	{
	private:
		uint64_t dataHigh;
		uint64_t dataLow;

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

		// Use explicit generation : Guid::Generate() or Guid::Empty
		Guid() = delete;

		Guid(const Guid& from) = default;

		constexpr Guid(uint64_t low, uint64_t high)
			: dataHigh(high), dataLow(low)
		{ }


		// Format this Guid to a sring
		// Format : 00000000-00000000-00000000-00000000
		std::string ToString()
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

		auto operator<=>(Guid const&) const = default;
	};

	inline Guid Guid::Empty = Guid(0,0);
}