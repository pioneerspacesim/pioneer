#ifndef _BEZIER_H
#define _BEZIER_H

#include "vector3.h"

struct QuadricBezier {
	vector3d p0, p1, p2;
	vector3d Eval(const double t) {
		return (1.0-t)*(1.0-t)*p0 +
			2.0*(1.0-t)*t*p1 +
			t*t*p2;
	}
};

struct CubicBezier {
	vector3d p0, p1, p2, p3;
	vector3d Eval(const double t) {
		return (1.0-t)*(1.0-t)*(1.0-t)*p0 +
			3.0*(1.0-t)*(1.0-t)*t*p1 +
			3.0*(1.0-t)*t*t*p2 +
			t*t*t*p3;
	}
	QuadricBezier DerivativeOf() {
		QuadricBezier d;
		d.p0 = 3.0*(p1 - p0);
		d.p1 = 3.0*(p2 - p1);
		d.p2 = 3.0*(p3 - p2);
		return d;
	}
};

struct QuarticBezier {
	vector3d p0, p1, p2, p3, p4;
	vector3d Eval(const double t) {
		return (1.0-t)*(1.0-t)*(1.0-t)*(1.0-t)*p0 + 
			4.0*(1.0-t)*(1.0-t)*(1.0-t)*t*p1 +
			6.0*(1.0-t)*(1.0-t)*t*t*p2 +
			4.0*(1.0-t)*t*t*t*p3 +
			t*t*t*t*p4;
	}
	CubicBezier DerivativeOf() {
		CubicBezier d;
		d.p0 = 4.0*(p1 - p0);
		d.p1 = 4.0*(p2 - p1);
		d.p2 = 4.0*(p3 - p2);
		d.p3 = 4.0*(p4 - p3);
		return d;
	}
};

#endif /* _BEZIER_H */
