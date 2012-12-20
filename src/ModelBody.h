// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MODELBODY_H
#define _MODELBODY_H

#include "Body.h"
#include "vector3.h"
#include "matrix4x4.h"
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
	virtual void SetRotMatrix(const matrix4x4d &r);
	vector3d GetPosition() const;
	virtual double GetBoundingRadius() const;
	void TransformToModelCoords(const Frame *camFrame);
	void GetRotMatrix(matrix4x4d &m) const;
	virtual void SetFrame(Frame *f);
	// to remove from simulation for a period
	virtual void Disable();
	virtual void Enable();
	void GetAabb(Aabb &aabb) const;
	Geom *GetGeom() { return m_geom; }
	ModelBase *GetModel() { return m_model; }
	CollMesh *GetCollMesh() { return m_collMesh.Get(); }
	LmrObjParams &GetLmrObjParams() { return m_params; }
	void SetLmrTimeParams();
	void RebuildCollisionMesh();

	void TriMeshUpdateLastPos(const matrix4x4d &currentTransform);
	void SetModel(const char *lmrModelName, bool isStatic = false);

	void RenderLmrModel(Graphics::Renderer *r, const vector3d &viewCoords, const matrix4x4d &viewTransform);

	virtual void UpdateInterpolatedTransform(double alpha);
protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);
private:
	bool m_isStatic;
	RefCountedPtr<CollMesh> m_collMesh;
	Geom *m_geom;
	LmrObjParams m_params;
	ModelBase *m_model;
};

#endif /* _MODELBODY_H */
