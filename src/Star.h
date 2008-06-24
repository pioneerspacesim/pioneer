#ifndef _STAR_H
#define _STAR_H

#include "Body.h"
#include "StarSystem.h"

class Frame;

class Star: public Body {
public:
	Star(StarSystem::SBody::SubType subtype);
	virtual ~Star() {};
	virtual void SetPosition(vector3d p);
	virtual vector3d GetPosition();
	void SetRadius(double radius) { m_radius = radius; }
	double GetRadius() { return m_radius; }
	virtual void Render(const Frame *camFrame);
	virtual void TransformToModelCoords(const Frame *camFrame);
	virtual void TransformCameraTo() {};
	
private:
	StarSystem::SBody::SubType m_subtype;
	vector3d m_pos;
	double m_radius;
};

#endif /* _STAR_H */
