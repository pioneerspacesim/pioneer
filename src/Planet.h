#ifndef _PLANET_H
#define _PLANET_H

#include "Body.h"
#include "StarSystem.h"

class Frame;

class Planet: public Body {
public:
	OBJDEF(Planet, Body, PLANET);
	Planet(StarSystem::SBody*);
	Planet() {}
	virtual ~Planet();
	virtual void SetPosition(vector3d p);
	virtual vector3d GetPosition() const;
	void SetRadius(double radius);
	virtual double GetRadius() const;
	virtual void Render(const Frame *camFrame);
	virtual void SetFrame(Frame *f);
	virtual bool OnCollision(Body *b, Uint32 flags) { return true; }
	virtual double GetMass() const { return m_mass; }
protected:
	virtual void Save();
	virtual void Load();
private:
	void Init();
	void DrawRockyPlanet();
	void DrawGasGiant();
	void DrawAtmosphere(double rad, vector3d &pos);

	double m_mass;
	vector3d pos;
	dGeomID geom;
	StarSystem::SBody *sbody;
	GLuint crudDList;
};

#endif /* _PLANET_H */
