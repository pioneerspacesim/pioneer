#ifndef _TERRAINBODY_H
#define _TERRAINBODY_H

#include "Body.h"
#include "galaxy/StarSystem.h"
#include "GeoSphere.h"
#include "Camera.h"

class Frame;
namespace Graphics { class Renderer; }

class TerrainBody : public Body {
public:
	OBJDEF(TerrainBody, Body, TERRAINBODY);

	virtual void SetPosition(vector3d pos) { m_pos = pos; }
	virtual vector3d GetPosition() const { return m_pos; }
	virtual double GetBoundingRadius() const;
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform);
	virtual void SubRender(Graphics::Renderer *r, const Camera *camera, const vector3d &camPos) {}
	virtual void SetFrame(Frame *f);
	virtual bool OnCollision(Object *b, Uint32 flags, double relVel) { return true; }
	virtual double GetMass() const { return m_mass; }
	double GetTerrainHeight(const vector3d pos) const;
	bool IsSuperType(SystemBody::BodySuperType t) const;
	virtual const SystemBody *GetSystemBody() const { return m_sbody; }
	GeoSphere *GetGeoSphere() const { return m_geosphere; }
	double GetMaxFeatureRadius() const { // returns value in metres
		return (m_geosphere->GetMaxFeatureHeight() + 1.0) * m_sbody->GetRadius();
	}

protected:
	TerrainBody(SystemBody*);
	TerrainBody();
	virtual ~TerrainBody();

	void InitTerrainBody(SystemBody *);

	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);

private:
	SystemBody *m_sbody;
	vector3d m_pos;
	double m_mass;
	GeoSphere *m_geosphere;
};

#endif
