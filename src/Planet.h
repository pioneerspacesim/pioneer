#ifndef _PLANET_H
#define _PLANET_H

#include "Body.h"
#include "StarSystem.h"

class Frame;

class Planet: public Body {
public:
	Planet(StarSystem::SBody::SubType);
	virtual ~Planet();
	virtual void SetPosition(vector3d p);
	virtual vector3d GetPosition();
	void SetRadius(double radius);
	double GetRadius() { return m_radius; }
	virtual void Render(const Frame *camFrame);
	virtual void TransformToModelCoords(const Frame *camFrame);
	virtual void TransformCameraTo() {};
	virtual void SetFrame(Frame *f);
	virtual bool OnCollision(Body *b) { return true; }
private:
	vector3d m_pos;
	double m_radius;
	dGeomID m_geom;
	StarSystem::SBody::SubType m_subtype;
};

#endif /* _PLANET_H */
