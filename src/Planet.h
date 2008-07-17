#ifndef _PLANET_H
#define _PLANET_H

#include "Body.h"
#include "StarSystem.h"

class Frame;

class Planet: public Body {
public:
	Planet(StarSystem::SBody*);
	virtual ~Planet();
	virtual void SetPosition(vector3d p);
	virtual vector3d GetPosition();
	void SetRadius(double radius);
	double GetRadius() { return radius; }
	virtual void Render(const Frame *camFrame);
	virtual void SetFrame(Frame *f);
	virtual bool OnCollision(Body *b, Uint32 flags) { return true; }
private:
	void DrawRockyPlanet();
	void DrawGasGiant();

	vector3d pos;
	double radius;
	dGeomID geom;
	StarSystem::SBody sbody;
};

#endif /* _PLANET_H */
