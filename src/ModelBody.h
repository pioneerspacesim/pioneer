// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MODELBODY_H
#define _MODELBODY_H

#include "Body.h"
#include "vector3.h"
#include "ModelBase.h"
#include "CollMesh.h"
#include "LmrTypes.h"
#include <vector>
class Geom;
namespace Graphics { class Renderer; }

class ModelBody: public Body {
public:
	OBJDEF(ModelBody, Body, MODELBODY);
	ModelBody();
	virtual ~ModelBody();
	void SetPosition(const vector3d &p);
	void SetOrient(const matrix3x3d &r);
	void TransformToModelCoords(const Frame *camFrame);
	void SetFrame(Frame *f);
	// Colliding: geoms are checked against collision space
	void SetColliding(bool colliding);
	bool IsColliding() const { return m_colliding; }
	// Static: geoms are static relative to frame
	void SetStatic(bool isStatic);
	bool IsStatic() const { return m_isStatic; }
	const Aabb &GetAabb() const { return m_collMesh->GetAabb(); }
	Geom *GetGeom() { return m_geom; }
	ModelBase *GetModel() { return m_model; }
	CollMesh *GetCollMesh() { return m_collMesh.Get(); }
	LmrObjParams &GetLmrObjParams() { return m_params; }
	void SetLmrTimeParams();
	void RebuildCollisionMesh();

	void SetModel(const char *lmrModelName);

	void RenderLmrModel(Graphics::Renderer *r, const vector3d &viewCoords, const matrix4x4d &viewTransform);

protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);
private:
	bool m_isStatic;
	bool m_colliding;
	RefCountedPtr<CollMesh> m_collMesh;
	Geom *m_geom;
	LmrObjParams m_params;
	ModelBase *m_model;
};

#endif /* _MODELBODY_H */
