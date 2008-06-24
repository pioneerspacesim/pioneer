#ifndef _RIGIDBODY_H
#define _RIGIDBODY_H

#include "Body.h"
#include "StaticRigidBody.h"
#include "vector3.h"
#include "matrix4x4.h"
class ObjMesh;

class RigidBody: public StaticRigidBody {
public:
	RigidBody();
	virtual ~RigidBody();
	void SetVelocity(vector3d v);
	void SetAngVelocity(vector3d v);
	void SetMesh(ObjMesh *m);
	virtual bool OnCollision(Body *b) { return true; }
	vector3d GetAngularMomentum();
	
	dBodyID m_body;
	dMass m_mass;
protected:
private:
	ObjMesh *m_mesh;
};

#endif /* _RIGIDBODY_H */
