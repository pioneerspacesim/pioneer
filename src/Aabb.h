// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _AABB_H
#define _AABB_H

#include "vector3.h"

#include <algorithm> // for std::min / std::max

struct Aabb {
	vector3d min, max;
	double radius;
	Aabb() :
		min(DBL_MAX, DBL_MAX, DBL_MAX),
		max(-DBL_MAX, -DBL_MAX, -DBL_MAX),
		radius(0.1)
	{
	}
	// Fast constructor for pre-conditioned values
	// Should only be used when _min < _max for all {x,y,z}
	Aabb(const vector3d &_min, const vector3d &_max, float rad) :
		min(_min),
		max(_max),
		radius(rad)
	{
	}
	void Update(const Aabb &b)
	{
		max.x = std::max(max.x, b.max.x);
		max.y = std::max(max.y, b.max.y);
		max.z = std::max(max.z, b.max.z);
		min.x = std::min(min.x, b.min.x);
		min.y = std::min(min.y, b.min.y);
		min.z = std::min(min.z, b.min.z);
		radius = std::max(radius, b.radius);
	}

	void Update(const vector3d &p)
	{
		if (max.x < p.x) max.x = p.x;
		if (max.y < p.y) max.y = p.y;
		if (max.z < p.z) max.z = p.z;
		if (min.x > p.x) min.x = p.x;
		if (min.y > p.y) min.y = p.y;
		if (min.z > p.z) min.z = p.z;
		if (p.Dot(p) > radius * radius) radius = p.Length();
	}
	void Update(float x, float y, float z)
	{
		if (max.x < x) max.x = x;
		if (max.y < y) max.y = y;
		if (max.z < z) max.z = z;
		if (min.x > x) min.x = x;
		if (min.y > y) min.y = y;
		if (min.z > z) min.z = z;
	}
	template <typename T>
	bool IsIn(const vector3<T> &p) const
	{
		return ((p.x >= min.x) && (p.x <= max.x) &&
			(p.y >= min.y) && (p.y <= max.y) &&
			(p.z >= min.z) && (p.z <= max.z));
	}
	bool Intersects(const Aabb &o) const
	{
		return min < o.max && max > o.min;
	}
	// returns maximum point radius, usually smaller than max radius of bounding box
	double GetRadius() const { return radius; }
};

// SIMD-capable AABB type without additional overhead
template <typename Vec3>
struct AABB {
	using Number = typename Vec3::element_type;

	Vec3 min, max;

	static AABB Invalid()
	{
		return AABB{ Vec3(DBL_MAX, DBL_MAX, DBL_MAX), Vec3(-DBL_MAX, -DBL_MAX, -DBL_MAX) };
	}

	void Update(const AABB &rhs)
	{
		using std::max;
		using std::min;

		this->max = max(this->max, rhs.max);
		this->min = min(this->min, rhs.min);
	}

	void Update(const Vec3 &point)
	{
		using std::max;
		using std::min;

		this->max = max(this->max, point);
		this->min = min(this->min, point);
	}

	auto IntersectsRay(const Vec3 &start, const Vec3 &inv_dir, Number dist) const
	{
		using std::max;
		using std::min;

		Vec3 l1 = (this->min - start) * inv_dir;
		Vec3 l2 = (this->max - start) * inv_dir;

		Vec3 vmin = min(l1, l2);
		Vec3 vmax = max(l1, l2);

		double lmin = max(vmin.x, max(vmin.y, vmin.z));
		double lmax = min(vmax.x, min(vmax.y, vmax.z));

		return ((lmax >= 0.f) & (lmax >= lmin) & (lmin < dist));
	}

	auto IsIn(Vec3 point) const { return point >= min && point <= max; }
	auto Intersects(const AABB &rhs) const { return min < rhs.max && max > rhs.min; }
};

using AABBd = AABB<vector3d>;

#endif /* _AABB_H */
