// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Sphere.h"

#include "Plane.h"

// Adapted from Ysaneya here: http://www.gamedev.net/blog/73/entry-1666972-horizon-culling/
///
///		Performs horizon culling with an object's bounding sphere, given a view point.
///		This function checks whether the object's sphere is inside the view cone formed
///		by the view point and this sphere. The view cone is capped so that the visibility
///		is false only in the 'shadow' of this sphere.
///		@param view Position of view point in world space
///		@param obj Bounding sphere of another object.
///		@return true if the object's bounding sphere is visible from the viewpoint, false if the
///		sphere is in the shadow cone AND not in front of the capping plane.
///
bool SSphere::HorizonCulling(const vector3d &view, const SSphere &obj) const
{
	vector3d O1C = m_centre - view;
	vector3d O2C = obj.m_centre - view;

	const double D1 = O1C.Length();
	const double D2 = O2C.Length();
	const double R1 = m_radius;
	const double R2 = obj.m_radius;
	const double iD1 = 1.0 / D1;
	const double iD2 = 1.0 / D2;

	O1C *= iD1;
	O2C *= iD2;
	const double K = O1C.Dot(O2C);

	const double K1 = R1 * iD1;
	const double K2 = R2 * iD2;
	bool status = true;
	if (K > K1 * K2) {
		status = (-2.0 * K * K1 * K2 + K1 * K1 + K2 * K2) < (1.0 - K * K);
	}

	const double y = R1 * R1 * iD1;
	const vector3d P = m_centre - y * O1C;
	const vector3d N = -O1C;
	const SPlane plane(N, P);
	status = status || (plane.DistanceToPoint(obj.m_centre) > obj.m_radius);

	return status;
}
