#ifndef _DYNAMICBODY_H
#define _DYNAMICBODY_H

#include "Body.h"
#include "ModelBody.h"
#include "vector3.h"
#include "matrix4x4.h"
class ObjMesh;

class DynamicBody: public ModelBody {
public:
	DynamicBody();
	virtual ~DynamicBody();
	virtual void SetRotation(const matrix4x4d &r);
	virtual void SetVelocity(vector3d v);
	virtual vector3d GetVelocity();
	void SetAngVelocity(vector3d v);
	void SetMesh(ObjMesh *m);
	virtual bool OnCollision(Body *b, Uint32 flags) { return true; }
	vector3d GetAngularMomentum();
	void SetMassDistributionFromCollMesh(const CollMesh *m);
	virtual void Disable();
	virtual void Enable();
	
	dBodyID m_body;
	dMass m_mass;
protected:
private:
	ObjMesh *m_mesh;
};

#endif /* _DYNAMICBODY_H */
