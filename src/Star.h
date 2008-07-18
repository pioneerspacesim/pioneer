#ifndef _STAR_H
#define _STAR_H

#include "Body.h"
#include "StarSystem.h"

class Frame;

class Star: public Body {
public:
	Star(StarSystem::BodyType type);
	virtual ~Star() {};
	virtual void SetPosition(vector3d p);
	virtual vector3d GetPosition();
	void SetRadius(double radius) { this->radius = radius; }
	virtual double GetRadius() const { return radius; }
	virtual void Render(const Frame *camFrame);
	
private:
	StarSystem::BodyType type;
	vector3d pos;
	double radius;
};

#endif /* _STAR_H */
