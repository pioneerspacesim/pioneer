#ifndef _PLANET_H
#define _PLANET_H

#include "Body.h"
// only for SBody::BodySuperType enum...
#include "StarSystem.h"

class Frame;
class SBody;
class GeoSphere;

class Planet: public Body {
public:
	OBJDEF(Planet, Body, PLANET);
	Planet(SBody*);
	Planet();
	virtual ~Planet();
	virtual void SetPosition(vector3d p);
	virtual vector3d GetPosition() const;
	void SetRadius(double radius);
	virtual double GetBoundingRadius() const;
	virtual void Render(const vector3d &viewCoords, const matrix4x4d &viewTransform);
	virtual void SetFrame(Frame *f);
	virtual bool OnCollision(Object *b, Uint32 flags, double relVel) { return true; }
	virtual double GetMass() const { return m_mass; }
	double GetTerrainHeight(const vector3d pos) const;
	void GetAtmosphericState(double dist, double *outPressure, double *outDensity);
	bool IsSuperType(SBody::BodySuperType t) const;
	virtual const SBody *GetSBody() const { return sbody; }
#if OBJECTVIEWER
	friend class ObjectViewerView;
#endif
protected:
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);
private:
	void Init();
	void DrawGasGiantRings();
	void DrawAtmosphere(vector3d &pos);

	double m_mass;
	vector3d pos;
	SBody *sbody;
	GLuint m_ringsDList;
	GeoSphere *m_geosphere;
};

#endif /* _PLANET_H */
