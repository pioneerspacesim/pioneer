// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#ifndef _PLANE_H
#define _PLANE_H

#include "vector3.h"

struct SPlane {
	double a, b, c, d;
	double DistanceToPoint(const vector3d &p) const;
	SPlane()
	{ /*default empty for Frustum*/
	}
	SPlane(const vector3d &N, const vector3d &P);
};

#endif /* _GEOPATCH_H */
