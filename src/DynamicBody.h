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
	virtual vector3d GetPosition() const;
	virtual vector3d GetVelocity();
	vector3d GetAngVelocity();
	void SetAngVelocity(vector3d v);
	void SetMesh(ObjMesh *m);
	virtual bool OnCollision(Body *b, Uint32 flags) { return true; }
	vector3d GetAngularMomentum();
	double GetAngularInertia() const { return m_angInertia; }
	void SetMassDistributionFromCollMesh(const CollMesh *m);
	void DisableBodyOnly() { m_enabled = false; }
	bool IsEnabled() { return m_enabled; }
	virtual void Disable();
	virtual void Enable();
	virtual double GetMass() const { return m_mass; }
	virtual void TimeStepUpdate(const float timeStep);
	void UndoTimestep();
	
	void SetMass(double);
	void AddForceAtPos(const vector3d force, const vector3d pos);
	void AddForce(const vector3d);
	void AddTorque(const vector3d);
	void SetForce(const vector3d);
	void SetTorque(const vector3d);
	// body-relative forces
	void AddRelForce(const vector3d);
	void AddRelTorque(const vector3d);

protected:
	virtual void Save();
	virtual void Load();
private:
	// new odeless turd
	matrix4x4d m_orient; // contains pos
	matrix4x4d m_oldOrient;
	vector3d m_force;
	vector3d m_torque;
	vector3d m_vel;
	vector3d m_angVel;
	double m_mass;
	double m_massRadius; // set in a mickey-mouse fashion from the collision mesh and used to calculate m_angInertia
	double m_angInertia; // always sphere mass distribution
	bool m_enabled;
};

#endif /* _DYNAMICBODY_H */
