#ifndef _NEWMODEL_ANIMATIONKEY_H
#define _NEWMODEL_ANIMATIONKEY_H

#include "vector3.h"
#include "Quaternion.h"

namespace Newmodel {

struct AnimationKey {
	double time;

	AnimationKey(double t) : time(t) { }
};

struct PositionKey : public AnimationKey {
	vector3f position;

	PositionKey(double t, const vector3f &pos)
	: AnimationKey(t)
	, position(pos) { }
};

struct RotationKey : public AnimationKey {
	Quaternionf rotation;

	RotationKey(double t, const Quaternionf &q)
	: AnimationKey(t)
	, rotation(q) { }
};

struct ScaleKey : public AnimationKey {
	vector3f scale;

	ScaleKey(double t, const vector3f &s)
	: AnimationKey(t)
	, scale(s) { }
};

}

#endif
