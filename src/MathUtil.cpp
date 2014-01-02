// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MathUtil.h"
#include "Pi.h"

namespace MathUtil {

vector3d RandomPointOnSphere(double minRadius, double maxRadius)
{
	// see http://mathworld.wolfram.com/SpherePointPicking.html
	// or a Google search for further information
	const double dist = Pi::rng.Double(minRadius, maxRadius);
	const double z = Pi::rng.Double_closed(-1.0, 1.0);
	const double theta = Pi::rng.Double(2.0*M_PI);
	const double r = sqrt(1.0 - z*z) * dist;
	return vector3d(r*cos(theta), r*sin(theta), z*dist);
}

vector3d RandomPointInCircle(double minRadius, double maxRadius)
{
	// m: minRadius, M: maxRadius, r: random radius
	// PDF(r) = 2/(M^2 - m^2) * r  for m <= r < M
	// CDF(r) = 1/(M^2 - m^2) * (r^2 - m^2)
	// per inversion method (http://en.wikipedia.org/wiki/Inversion_method): CDF(r) := Uniform{0..1}
	// r = sqrt(Uniform{0..1} * (M^2 - m^2) + m^2) = sqrt(Uniform{m^2..M^2})
	const double r = sqrt(Pi::rng.Double(minRadius*minRadius, maxRadius*maxRadius));
	const double phi = Pi::rng.Double(2.0*M_PI);
	return vector3d(r*cos(phi), r*sin(phi), 0.0);
}

}
