#pragma once


namespace RexEngine
{
	struct Color
	{
		using byte = unsigned char;
		float r, g, b, a;

		Color() : Color(0.0f, 0.0f, 0.0f, 0.0f) {}
		Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

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