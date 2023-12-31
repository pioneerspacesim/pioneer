// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Plane.h"

double SPlane::DistanceToPoint(const vector3d &p) const
{
	return a * p.x + b * p.y + c * p.z + d;
}

SPlane::SPlane(const vector3d &N, const vector3d &P)
{
	const vector3d NormalizedNormal = N.Normalized();
	a = NormalizedNormal.x;
	b = NormalizedNormal.y;
	c = NormalizedNormal.z;
	d = -(P.Dot(NormalizedNormal));
}
