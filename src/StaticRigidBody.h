#ifndef _STATICRIGIDBODY_H
#define _STATICRIGIDBODY_H

#include "Body.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "sbre/sbre.h"
class ObjMesh;

class StaticRigidBody: public Body {
public:
	StaticRigidBody();
	virtual ~StaticRigidBody();
	void SetPosition(vector3d p);
	// not valid to do SetVelocity on these. they are for huge things like
	// space stations and will be static relative to their frame of reference
	void SetVelocity(vector3d v);
	vector3d GetPosition();
	void TransformToModelCoords(const Frame *camFrame);
	void TransformCameraTo();
	void ViewingRotation();
	void GetRotMatrix(matrix4x4d &m);
	virtual void SetFrame(Frame *f);

	void RenderSbreModel(const Frame *camFrame, int model, ObjParams *params);
protected:
	dGeomID m_geom;
};

#endif /* _STATICRIGIDBODY_H */
