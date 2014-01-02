// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_POINT_H
#define UI_POINT_H

#include "vector2.h"

namespace UI {

class Point {
public:
	int x, y;

	Point() : x(0), y(0) {}
	Point(int _x, int _y) : x(_x), y(_y) {}
	explicit Point(int v) : x(v), y(v) {}
	Point(const Point &p) : x(p.x), y(p.y) {}

	Point operator+(const Point &v) const { return Point(x+v.x,y+v.y); }
	Point operator-(const Point &v) const { return Point(x-v.x,y-v.y); }
	Point &operator+=(const Point &v) { x+=v.x; y+=v.y; return *this; }
	Point &operator-=(const Point &v) { x-=v.x; y-=v.y; return *this; }
	Point &operator*=(const int &a) { x*=a; y*=a; return *this; }
	Point operator-() const { return Point(-x,-y); }

	friend Point operator*(const Point &v, const int &a) { return Point(v.x*a, v.y*a); }
	friend Point operator*(const int &a, const Point &v) { return v*a; }
	friend Point operator/(const Point &v, const int &a) { return Point(v.x/a, v.y/a); }
	friend bool operator==(const Point &a, const Point &b) { return a.x == b.x && a.y == b.y; }
	friend bool operator!=(const Point &a, const Point &b) { return ! (a == b); }

	enum Component { X, Y };
	const int &operator[](Component c) const { return c == X ? x : y; }
	int &operator[](Component c) { return c == X ? x : y; }
};

}

#endif
