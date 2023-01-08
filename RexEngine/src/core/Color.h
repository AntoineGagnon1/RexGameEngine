#pragma once

#include "../math/Vectors.h"

namespace RexEngine
{
	struct Color
	{
		using byte = unsigned char;
		float r, g, b, a;

		constexpr Color() : Color(0.0f, 0.0f, 0.0f, 0.0f) {}
		constexpr Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

		static Color FromRGBA32(byte r, byte g, byte b, byte a = 255)
		{
			return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
		}

		Color operator*=(const Color& rhs)
		{
			r *= rhs.r;
			g *= rhs.g;
			b *= rhs.b;
			a *= rhs.a;

			return *this;
		}

		Color operator*=(float rhs)
		{
			r *= rhs;
			g *= rhs;
			b *= rhs;
			a *= rhs;

			return *this;
		}

		explicit operator Vector3() const { return Vector3(r, g, b); }
		explicit operator Vector4() const { return Vector4(r, g, b, a); }

		template<typename Archive>
		void serialize(Archive& archive)
		{
			// Serialize as an array of 4 floats
			cereal::size_type s = 4;
			archive(cereal::make_size_tag(s));
			archive(r, g, b, a);
		}
	};

	inline Color operator*(Color lhs, const Color& rhs)
	{
		return lhs *= rhs;
	}

	inline Color operator*(Color lhs, float rhs)
	{
		return lhs *= rhs;
	}
}