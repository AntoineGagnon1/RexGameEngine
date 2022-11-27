#pragma once

#include <type_traits>

namespace RexEngine
{
	template<size_t Size, size_t Size2>
	concept IsEqual = requires() { Size == Size2; };

	template<size_t Size, size_t MinSize>
	concept IsGreaterOrEqual = requires() { Size >= MinSize; };

	template <typename T, typename base>
	concept IsDerivedFrom = std::is_base_of<base, T>::value;
}