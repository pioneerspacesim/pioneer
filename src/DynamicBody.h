#ifndef _DYNAMICBODY_H
#define _DYNAMICBODY_H

#include "Body.h"
#include "ModelBody.h"
#include "vector3.h"
#include "matrix4x4.h"

class DynamicBody: public ModelBody {
public:
	OBJDEF(DynamicBody, ModelBody, DYNAMICBODY);
	DynamicBody();
	virtual ~DynamicBody();
	virtual void SetRotMatrix(const matrix4x4d &r);
	virtual void GetRotMatrix(matrix4x4d &m);
	virtual void SetVelocity(vector3d v);
	virtual void SetPosition(vector3d p);
	virtual vector3d GetVelocity();
	vector3d GetAngVelocity();
	void SetAngVelocity(vector3d v);
	void SetMesh(ObjMesh *m);
	virtual bool OnCollision(Body *b, Uint32 flags) { return true; }
	vector3d GetAngularMomentum();
	void SetMassDistributionFromCollMesh(const CollMesh *m);
	virtual void Disable();
	virtual void Enable();
	virtual double GetMass() const { return m_mass.mass; }
	virtual void TimeStepUpdate(const float timeStep);
	
	dBodyID m_body;
	dMass m_mass;
protected:
	virtual void Save();
	virtual void Load();
private:
};

#endif /* _DYNAMICBODY_H */
