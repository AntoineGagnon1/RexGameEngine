#pragma once

#include "glm/gtx/quaternion.hpp"

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

		// TODO : static FromEuler(vec3)

		float Magnitude() const
		{
			return glm::length((GlmType)*this);
		}

		Quaternion Normalized() const
		{
			return glm::normalize((GlmType)*this);
		}

		operator GlmType() const { return *this; }
	};
}