#ifndef HEATGRADIENTPAR_H_INCLUDED
#define HEATGRADIENTPAR_H_INCLUDED

#include "matrix3x3.h"
#include "vector3.h"

struct HeatGradientParameters_t {
	matrix3x3f heatingMatrix;
	vector3f heatingNormal; // normalised
	float heatingAmount; // 0.0 to 1.0 used for `u` component of heatGradient texture
};


#endif // HEATGRADIENTPAR_H_INCLUDED
