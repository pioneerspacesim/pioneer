#ifndef ATMOSPHEREPARAMETERS_H_INCLUDED
#define ATMOSPHEREPARAMETERS_H_INCLUDED

#include "Color.h"
#include "vector3.h"

struct AtmosphereParameters {
	float atmosRadius;
	float atmosInvScaleHeight;
	float atmosDensity;
	float planetRadius;
	Color atmosCol;
	vector3d center;
};

#endif // ATMOSPHEREPARAMETERS_H_INCLUDED
