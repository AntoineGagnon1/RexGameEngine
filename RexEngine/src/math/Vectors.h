#pragma once

#include <initializer_list>

namespace RexEngine
{

	template<typename T>
	struct _Vector2
	{
		T x, y;

		_Vector2(T x, T y) : x(x), y(y) {}
		_Vector2() : _Vector2(0,0) {}

		_Vector2(const _Vector2&) = default;

		friend bool operator==(const _Vector2&, const _Vector2&) = default;
	};

	template<typename T>
	struct _Vector3
	{
		T x, y, z;

		_Vector3(_Vector2<T> xy, T z) : _Vector3(xy.x, xy.y, z) {}
		_Vector3(T x, T y, T z) : x(x), y(y), z(z) {}
		_Vector3() : _Vector3(0, 0, 0) {}

		_Vector3(const _Vector3&) = default;

		friend bool operator==(const _Vector3&, const _Vector3&) = default;
	};

	template<typename T>
	struct _Vector4
	{
		T x, y, z, w;

		_Vector4(_Vector2<T> xy, T z, T w) : _Vector4(xy.x, xy.y, z, w) {}
		_Vector4(_Vector2<T> xy, _Vector2<T> zw) : _Vector4(xy.x, xy.y, zw.z, zw.w) {}
		_Vector4(_Vector3<T> xyz, T w) : _Vector4(xyz.x, xyz.y, xyz.z, w) {}
		_Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
		_Vector4() : _Vector4(0, 0, 0, 0) {}

		_Vector4(const _Vector4&) = default;

		friend bool operator==(const _Vector4&, const _Vector4&) = default;
	};

	using Vector2 = _Vector2<float>;
	using Vector2Int = _Vector2<int>;

	using Vector3 = _Vector3<float>;
	using Vector3Int = _Vector3<int>;

	using Vector4 = _Vector4<float>;
	using Vector4Int = _Vector4<int>;
}