// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _AABB_H
#define _AABB_H

#include "libs.h"

struct Aabb {
	vector3d min, max;
	double radius;
	Aabb()
		: min(DBL_MAX, DBL_MAX, DBL_MAX)
		, max(-DBL_MAX, -DBL_MAX, -DBL_MAX)
		, radius(0.1)
	{ }
	void Update(const vector3d &p) {
		if (max.x < p.x) max.x = p.x;
		if (max.y < p.y) max.y = p.y;
		if (max.z < p.z) max.z = p.z;
		if (min.x > p.x) min.x = p.x;
		if (min.y > p.y) min.y = p.y;
		if (min.z > p.z) min.z = p.z;
		if (p.Dot(p) > radius*radius) radius = p.Length();
	}
	void Update(float x, float y, float z) {
		if (max.x < x) max.x = x;
		if (max.y < y) max.y = y;
		if (max.z < z) max.z = z;
		if (min.x > x) min.x = x;
		if (min.y > y) min.y = y;
		if (min.z > z) min.z = z;
	}
	template <typename T>
	bool IsIn (const vector3<T> &p) const {
		return ((p.x >= min.x) && (p.x <= max.x) &&
		    (p.y >= min.y) && (p.y <= max.y) &&
		    (p.z >= min.z) && (p.z <= max.z));
	}
	bool Intersects(const Aabb &o) const {
		return (min.x < o.max.x) && (max.x > o.min.x) &&
			(min.y < o.max.y) && (max.y > o.min.y) &&
			(min.z < o.max.z) && (max.z > o.min.z);
	}
	// returns maximum point radius, usually smaller than max radius of bounding box
	double GetRadius() const { return radius; }
};

#endif /* _AABB_H */
