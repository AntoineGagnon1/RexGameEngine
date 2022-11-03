#pragma once

#include <type_traits>


namespace RexEngine
{
	class Scalar
	{
	public:

		template<typename T>
		static std::type_identity_t<T> Min(T value, std::type_identity_t<T> max)
		{
			return (value < max ? value : max);
		}

		template<typename T>
		static std::type_identity_t<T> Max(T value, std::type_identity_t<T> min)
		{
			return (value > min ? value : min);
		}


		template<typename T>
		static std::type_identity_t<T> Clamp(T value, std::type_identity_t<T> min, std::type_identity_t<T> max)
		{
			return Max(Min(value, max), min);
		}
	};
}