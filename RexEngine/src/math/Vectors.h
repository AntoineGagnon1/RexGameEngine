#pragma once

#define GLM_FORCE_SWIZZLE // .xy, .xyz, ...
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/geometric.hpp>

#include "../utils/Concepts.h"


namespace RexEngine
{
	template<typename T, size_t Size>
	struct Vector : public glm::vec<Size, T>
	{
		using GlmType = glm::vec<Size, T>;
		using VecType = Vector<T, Size>;

		// General constructors
		constexpr Vector() = default;
		constexpr Vector(const GlmType& from) : GlmType(from) {}
		constexpr Vector(const VecType& from) = default;
		
		// Size specific constructors
		template<typename ...Args>
		constexpr Vector(Args&&... args) : GlmType(args...) {}

		// Vector 3
		constexpr explicit Vector(Vector<T, 2> xy, T z) requires IsEqual<Size, 3> : GlmType(xy, z) {}

		// Vector 4
		constexpr explicit Vector(Vector<T, 3> xyz, T w) requires IsEqual<Size, 4> : GlmType(xyz, w) {}
		constexpr explicit Vector(Vector<T, 2> xy, T z, T w) requires IsEqual<Size, 4> : GlmType(xy.x(), xy.y(), z, w) {}
		constexpr explicit Vector(Vector<T, 2> xy, Vector<T, 2> zw) requires IsEqual<Size, 4> : GlmType(xy.x(), xy.y(), zw.z(), zw.w()) {}

		T SqrMagnitude() const
		{
			T result = static_cast<T>(0);
			for (size_t i = 0; i < Size; i++)
				result += GlmType::operator[](i) * GlmType::operator[](i);

			return result;
		}

		T Magnitude() const
		{
			return static_cast<T>(sqrt(SqrMagnitude()));
		}

		VecType Normalized() const
		{
			return glm::normalize(*this);
		}

		T Dot(const VecType& rhs) const
		{
			return glm::dot((*this), rhs);
		}

		VecType Cross(const VecType& rhs) const
		{
			return glm::cross((*this), rhs);
		}

		// You probably want Magnitude()
		static int length() = delete;
		operator GlmType() const { return *this; }
	};

	using Vector2 = Vector<float, 2>;
	using Vector2Int = Vector<int, 2>;

	using Vector3 = Vector<float, 3>;
	using Vector3Int = Vector<int, 3>;
	
	using Vector4 = Vector<float, 4>;
	using Vector4Int = Vector<int, 4>;

	class Directions
	{
	public:
		inline static Vector3 Up = Vector3(0,1,0);
		inline static Vector3 Down = Vector3(0,-1,0);

		inline static Vector3 Right = Vector3(1,0,0);
		inline static Vector3 Left = Vector3(-1,0,0);
		
		inline static Vector3 Forward = Vector3(0,0,1);
		inline static Vector3 Backward = Vector3(0,0,-1);
	};
}