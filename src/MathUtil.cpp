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

}
