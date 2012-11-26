// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MODELBODY_H
#define _MODELBODY_H

#include "Body.h"
#include "vector3.h"
#include "matrix3x3.h"
#include "LmrModel.h"
#include <vector>
class Geom;

class ModelBody: public Body {
public:
	OBJDEF(ModelBody, Body, MODELBODY);
	ModelBody();
	virtual ~ModelBody();
	void SetPosition(const vector3d &p);
	void SetOrient(const matrix3x3d &r);
	virtual double GetPhysRadius() const;
	virtual double GetClipRadius() const;
	void TransformToModelCoords(const Frame *camFrame);
	void SetFrame(Frame *f);
	// Colliding: geoms are checked against collision space
	void SetColliding(bool colliding);
	bool IsColliding() const { return m_colliding; }
	// Static: geoms are static relative to frame
	void SetStatic(bool isStatic);
	bool IsStatic() const { return m_isStatic; }
	const Aabb &GetAabb() const;
	Geom *GetGeom() { return m_geom; }
	LmrModel *GetLmrModel() { return m_lmrModel; }
	LmrCollMesh *GetLmrCollMesh() { return m_collMesh; }
	LmrObjParams &GetLmrObjParams() { return m_params; }
	void SetLmrTimeParams();
	void RebuildCollisionMesh();

	void TriMeshUpdateLastPos(const matrix4x4d &currentTransform);
	void SetModel(const char *lmrModelName);

	void RenderLmrModel(const vector3d &viewCoords, const matrix4x4d &viewTransform);

//	virtual void UpdateInterpolatedTransform(double alpha);		// no different from body.h
protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);
private:
	LmrModel *m_lmrModel;
	LmrCollMesh *m_collMesh;
	LmrObjParams m_params;
	bool m_isStatic;
	bool m_colliding;
	Geom *m_geom;
};

#endif /* _MODELBODY_H */
