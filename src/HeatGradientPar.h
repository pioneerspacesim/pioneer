#ifndef HEATGRADIENTPAR_H_INCLUDED
#define HEATGRADIENTPAR_H_INCLUDED

struct HeatGradientParameters_t {
	vector3f heatingNormal; // normalised
	float heatingAmount;	// 0.0 to 1.0 used for `u` component of heatGradient texture
};

#endif // HEATGRADIENTPAR_H_INCLUDED
