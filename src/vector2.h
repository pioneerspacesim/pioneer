// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _VECTOR2_H
#define _VECTOR2_H

#include "FloatComparison.h"
#include <math.h>

template <typename T>
struct other_floating_type {};
template <>
struct other_floating_type<float> {
	typedef double type;
};
template <>
struct other_floating_type<double> {
	typedef float type;
};

template <typename T>
class vector2 {
public:
	T x, y;

	vector2() :
		x(0.0f),
		y(0.0f) {}
	vector2(T _x, T _y) :
		x(_x),
		y(_y) {}
	explicit vector2(int v) :
		x(T(v)),
		y(T(v)) {}
	explicit vector2(unsigned int v) :
		x(T(v)),
		y(T(v)) {}
	explicit vector2(T v) :
		x(v),
		y(v) {}
	explicit vector2(const T v[2]) :
		x(v[0]),
		y(v[1]) {}

	// disallow implicit conversion between floating point sizes
	explicit vector2(const vector2<typename other_floating_type<T>::type> &v);
	explicit vector2(const typename other_floating_type<T>::type vals[2]);

	vector2 operator+(const vector2 &v) const { return vector2(x + v.x, y + v.y); }
	vector2 operator-(const vector2 &v) const { return vector2(x - v.x, y - v.y); }
	vector2 &operator+=(const vector2 &v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}
	vector2 &operator-=(const vector2 &v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}
	vector2 &operator*=(const T &a)
	{
		x *= a;
		y *= a;
		return *this;
	}
	vector2 operator-() const { return vector2(-x, -y); }

	bool operator==(const vector2 &a) const
	{
		return is_equal_exact(a.x, x) && is_equal_exact(a.y, y);
	}
	bool ExactlyEqual(const vector2 &a) const
	{
		return is_equal_exact(a.x, x) && is_equal_exact(a.y, y);
	}

	inline T &operator[](unsigned int idx) { return (&x)[idx]; }

	friend vector2 operator*(const vector2 &v, const T &a) { return vector2(v.x * a, v.y * a); }
	friend vector2 operator*(const T &a, const vector2 &v) { return v * a; }
	friend vector2 operator*(const vector2 &va, const vector2 &vb) { return vector2(va.x * vb.x, va.y * vb.y); }
	friend vector2 operator/(const vector2 &v, const T &a) { return vector2(v.x / a, v.y / a); }
	friend vector2 operator/(const vector2 &va, const vector2 &vb) { return vector2(va.x / vb.x, va.y / vb.y); }
	friend bool operator<(const vector2 &va, const vector2 &vb) { return va.LengthSqr() < vb.LengthSqr(); }

	T Length() const { return sqrt(x * x + y * y); }
	T LengthSqr() const { return x * x + y * y; }
	vector2 Normalized() const
	{
		const T invlen = 1.0f / sqrt(x * x + y * y);
		return vector2(x * invlen, y * invlen);
	}
	vector2 NormalizedSafe() const
	{
		const T lenSqr = x * x + y * y;
		if (lenSqr < 1e-18) // sqrt(lenSqr) < 1e-9
			return vector2(1, 0);
		else {
			const T invlen = sqrt(lenSqr);
			return vector2(x / invlen, y / invlen);
		}
	}
	vector2 Rotate(T alpha) // Rotate around center
	{
		return vector2(x * cos(alpha) - y * sin(alpha), y * cos(alpha) + x * sin(alpha));
	}

	void Print() const { printf("v(%f,%f)\n", x, y); }
};

template <>
inline vector2<float>::vector2() :
	x(0.f),
	y(0.f) {}
template <>
inline vector2<double>::vector2() :
	x(0.),
	y(0.) {}
template <>
inline vector2<float>::vector2(const vector2<double> &v) :
	x(float(v.x)),
	y(float(v.y)) {}
template <>
inline vector2<double>::vector2(const vector2<float> &v) :
	x(v.x),
	y(v.y) {}

typedef vector2<float> vector2f;
typedef vector2<double> vector2d;

#endif
