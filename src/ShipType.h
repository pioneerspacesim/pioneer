#ifndef _SHIPTYPE_H
#define _SHIPTYPE_H

#include "libs.h"
#include "vector3.h"

struct ShipType {
	public:
	enum Thruster { THRUSTER_FRONT, THRUSTER_REAR, THRUSTER_TOP, THRUSTER_BOTTOM, THRUSTER_LEFT, THRUSTER_RIGHT, THRUSTER_MAX };
	enum Type { COBRA3 };
	enum { GUNMOUNT_MAX = 2 };
	
	////////
	float linThrust[THRUSTER_MAX];
	float angThrust;
	struct GunMount {
		vector3f pos;
		vector3f dir;
	} gunMount[GUNMOUNT_MAX];
	///////

	static const ShipType types[1];
};

#endif /* _SHIPTYPE_H */
