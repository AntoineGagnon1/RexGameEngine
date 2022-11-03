#pragma once

#include "glm/gtx/quaternion.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"

#include "Vectors.h"

namespace RexEngine
{
	struct Quaternion : public glm::quat
	{
		using GlmType = glm::quat;

		constexpr Quaternion() = default;
		constexpr Quaternion(const GlmType& from) : GlmType(from) {}
		constexpr Quaternion(const Quaternion& from) = default;
		explicit constexpr Quaternion(float x, float y, float z, float w) : GlmType(w, x, y, z) {}
		explicit constexpr Quaternion(float scalar, Vector3 axis) : GlmType(scalar, axis) {}

		// Angles in degrees
		inline constexpr static Quaternion FromEuler(Vector3 eulers)
		{
			return glm::quat(glm::vec3(glm::radians(eulers.x), glm::radians(eulers.y), glm::radians(eulers.z)));
		}

		// Angle in degrees, axis doesnt have to be normalized
		inline constexpr static Quaternion AngleAxis(float angle, Vector3 axis)
		{
			return Quaternion(glm::angleAxis(glm::radians(angle), axis.Normalized()));
		}

		float Magnitude() const
		{
			return glm::length((GlmType)*this);
		}

		Quaternion Normalized() const
		{
			return glm::normalize((GlmType)*this);
		}


		// Member operators need to defined
		Quaternion operator*=(const Quaternion& rhs)
		{
			return (*this = (*this) * rhs);
		}

		operator GlmType() const { return *this; }
	};
}