// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "vector3.h"

/**
 * Simple oriented-plane helper class
 */
template<typename T>
struct Plane {
	Plane()
	{
	}

	Plane(const vector3<T> &N, const vector3<T> &P)
	{
		const vector3<T> nn = N.Normalized();
		a = nn.x;
		b = nn.y;
		c = nn.z;
		d = -(P.Dot(nn));
	}

	T DistanceToPoint(const vector3<T> &p) const
	{
		return a * p.x + b * p.y + c * p.z + d;
	}

	T a, b, c, d;
};
