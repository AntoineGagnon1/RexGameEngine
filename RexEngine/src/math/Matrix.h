#pragma once

#include <concepts>
#include <tuple>

#pragma warning(push, 0)
#define GLM_SILENT_WARNINGS GLM_ENABLE
#include <glm/matrix.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#pragma warning(pop)

#include "Vectors.h"
#include "Quaternion.h"
#include "../utils/Concepts.h"

namespace RexEngine
{
	template<typename T, size_t Size>
	struct Matrix : glm::mat<Size, Size, T>
	{
		using GlmType = glm::mat<Size, Size, T>;
		using MatType = Matrix<T, Size>;
		using VecType = Vector<T, Size>;

		static const MatType Identity;

		constexpr Matrix() = default;
		constexpr Matrix(const GlmType& from) : GlmType(from) {}
		constexpr Matrix(const MatType& from) = default;
		
		
		// Returns <Position, Rotation, Scale>
		std::tuple<Vector3, Quaternion, Vector3> Decompose() const requires IsEqual<Size, 4>
		{
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;

			glm::vec3 skew; // Not used
			glm::vec4 perspective; // Not used
			
			glm::decompose(*this, scale, rotation, translation, skew, perspective);
			return {translation, glm::conjugate(rotation), scale}; // The rotation returned by glm is inverted for some reason
		}

		// The position component of a 4x4 matrix
		Vector3 Position() const requires IsEqual<Size, 4>
		{
			auto vec = operator[](3);
			return Vector3(vec.x, vec.y, vec.z);
		}

		Quaternion Rotation() const requires IsEqual<Size, 4>
		{ // TODO : optimise this (get only the rotation)
			auto&& [_, rot, __] = Decompose();
			return rot;
		}

		MatType Inversed() const
		{
			return glm::inverse(*this);
		}

		MatType Transposed() const
		{
			return glm::transpose(*this);
		}

		VecType& operator[](size_t i)
		{
			return (VecType&)GlmType::operator[](static_cast<GlmType::length_type>(i));
		}

		const VecType& operator[](size_t i) const
		{
			return (const VecType&)GlmType::operator[](static_cast<GlmType::length_type>(i));
		}

		inline static MatType MakeTransform(const Vector3& translate, const Quaternion& rotation, const Vector3& scale) requires IsEqual<Size, 4>
		{
			return glm::translate(glm::mat4(1.0f), translate) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
		}

		// fov in degrees, aspect is x/y
		inline static MatType MakePerspective(float fov, float aspect, float zNear, float zFar) requires IsEqual<Size, 4>
		{
			return glm::perspective(glm::radians(fov), aspect, zNear, zFar);
		}

		inline static MatType MakeLookAt(Vector<T, 3> eye, Vector<T, 3> lookAt, Vector<T, 3> up) requires IsEqual<Size, 4>
		{
			return glm::lookAt(eye, lookAt, up);
		}

		template<typename Archive>
		void serialize(Archive& archive)
		{
			cereal::size_type s = Size;
			archive(cereal::make_size_tag(s));
			for (int i = 0; i < Size; i++)
				archive(operator[](i));
		}
	};

	using Matrix3 = Matrix<float, 3>;
	using Matrix4 = Matrix<float, 4>;

	inline const Matrix4 Matrix4::Identity = GlmType(static_cast<float>(1));
}