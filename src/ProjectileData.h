#ifndef PROJECTILEDATA_H_INCLUDED
#define PROJECTILEDATA_H_INCLUDED

#include "Color.h"

struct ProjectileData {
	ProjectileData() :
		lifespan(0.0f),
		damage(0.0f),
		length(0.0f),
		width(0.0f),
		speed(0.0f),
		color(Color::WHITE),
		mining(false),
		beam(false) {}
	ProjectileData(float ls, float d, float l, float w, float s, Color c, bool m, bool b) :
		lifespan(ls),
		damage(d),
		length(l),
		width(w),
		speed(s),
		color(c),
		mining(m),
		beam(b) {}
	float lifespan;
	float damage;
	float length;
	float width;
	float speed;
	Color color;
	bool mining;
	bool beam;
};


#endif // PROJECTILEDATA_H_INCLUDED
