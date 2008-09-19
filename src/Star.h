#ifndef _STAR_H
#define _STAR_H

#include "Body.h"
#include "StarSystem.h"

class Frame;

class Star: public Body {
public:
	OBJDEF(Star, Body, STAR);
	Star(StarSystem::SBody *sbody);
	Star() {}
	virtual ~Star() {};
	virtual void SetPosition(vector3d p);
	virtual vector3d GetPosition() const;
	virtual double GetRadius() const { return radius; }
	virtual void Render(const Frame *camFrame);
	virtual double GetMass() const { return mass; }
protected:
	virtual void Save();
	virtual void Load();
private:
	StarSystem::BodyType type;
	vector3d pos;
	double radius;
	double mass;
};

#endif /* _STAR_H */
