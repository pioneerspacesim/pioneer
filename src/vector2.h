#ifndef _VECTOR2_H
#define _VECTOR2_H

#include "FloatComparison.h"

class vector2f {
public:
	float x,y;

	vector2f() : x(0.0f), y(0.0f) {}
	vector2f(float _x, float _y) : x(_x), y(_y) {}
	vector2f(int v) : x(float(v)), y(float(v)) {}
	vector2f(float v) : x(v), y(v) {}
	vector2f(const vector2f &v) : x(v.x), y(v.y) {}
	vector2f(const float v[2]) : x(v[0]), y(v[1]) {}

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

	enum Component { X, Y };
	const float &operator[](Component c) const { return c == X ? x : y; }
	float &operator[](Component c) { return c == X ? x : y; }
};

#endif
