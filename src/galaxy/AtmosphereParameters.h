#ifndef ATMOSPHEREPARAMETERS_H_INCLUDED
#define ATMOSPHEREPARAMETERS_H_INCLUDED

#include "Color.h"
#include "vector3.h"

#define DENSITY_STEPS 255

struct AtmosphereParameters {
	float atmosRadius;
	float atmosInvScaleHeight;
	float atmosDensity;
	float planetRadius;
	Color atmosCol;
	vector3d center;
	vector2f logDensityMap[DENSITY_STEPS + 1];
	float logDensityMapR[(DENSITY_STEPS + 1)*(DENSITY_STEPS + 1)];
	float logDensityMapM[(DENSITY_STEPS + 1)*(DENSITY_STEPS + 1)];
};

#endif // ATMOSPHEREPARAMETERS_H_INCLUDED
