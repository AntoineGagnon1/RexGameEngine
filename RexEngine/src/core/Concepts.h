#pragma once

namespace RexEngine
{
	template<size_t Size, size_t Size2>
	concept IsEqual = requires() { Size == Size2; };

	template<size_t Size, size_t MinSize>
	concept IsGreaterOrEqual = requires() { Size >= MinSize; };
}