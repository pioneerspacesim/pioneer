// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _VECTOR2_H
#define _VECTOR2_H

#include "FloatComparison.h"
#include <math.h>

class vector2f {
public:
	float x,y;

	vector2f() : x(0.0f), y(0.0f) {}
	vector2f(float _x, float _y) : x(_x), y(_y) {}
	explicit vector2f(int v) : x(float(v)), y(float(v)) {}
	explicit vector2f(unsigned int v) : x(float(v)), y(float(v)) {}
	explicit vector2f(float v) : x(v), y(v) {}
	vector2f(const vector2f &v) : x(v.x), y(v.y) {}
	explicit vector2f(const float v[2]) : x(v[0]), y(v[1]) {}

	vector2f operator+(const vector2f &v) const { return vector2f(x+v.x,y+v.y); }
	vector2f operator-(const vector2f &v) const { return vector2f(x-v.x,y-v.y); }
	vector2f &operator+=(const vector2f &v) { x+=v.x; y+=v.y; return *this; }
	vector2f &operator-=(const vector2f &v) { x-=v.x; y-=v.y; return *this; }
	vector2f &operator*=(const float &a) { x*=a; y*=a; return *this; }
	vector2f operator-() const { return vector2f(-x,-y); }

	bool ExactlyEqual(const vector2f &a) const {
		return is_equal_exact(a.x, x) && is_equal_exact(a.y, y);
	}

	friend vector2f operator*(const vector2f &v, const float &a) { return vector2f(v.x*a, v.y*a); }
	friend vector2f operator*(const float &a, const vector2f &v) { return v*a; }
	friend vector2f operator/(const vector2f &v, const float &a) { return vector2f(v.x/a, v.y/a); }

	float Length() const { return sqrt(x*x + y*y); }
	float LengthSqr() const { return x*x + y*y; }
	vector2f Normalized() const { const float invlen = 1.0f / sqrt(x*x + y*y); return vector2f(x*invlen, y*invlen); }
	vector2f NormalizedSafe() const {
		const float lenSqr = x*x + y*y;
		if (lenSqr < 1e-18) // sqrt(lenSqr) < 1e-9
			return vector2f(1,0);
		else {
			const float invlen = sqrt(lenSqr);
			return vector2f(x/invlen, y/invlen);
		}
	}
};

#endif
