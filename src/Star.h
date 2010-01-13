#ifndef _STAR_H
#define _STAR_H

#include "Body.h"
#include "StarSystem.h"

class Frame;

class Star: public Body {
public:
	OBJDEF(Star, Body, STAR);
	Star(SBody *sbody);
	Star() {}
	virtual ~Star() {};
	virtual void SetPosition(vector3d p);
	virtual vector3d GetPosition() const;
	virtual double GetBoundingRadius() const { return radius*2.0f; }
	virtual void Render(const vector3d &viewCoords, const matrix4x4d &viewTransform);
	virtual double GetMass() const { return mass; }
protected:
	virtual void Save();
	virtual void Load();
private:
	SBody::BodyType type;
	vector3d pos;
	double radius;
	double mass;
};

#endif /* _STAR_H */
