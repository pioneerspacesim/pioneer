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
	virtual void GetRotMatrix(matrix4x4d &m) const;
	virtual void SetVelocity(vector3d v);
	virtual void SetPosition(vector3d p);
	virtual vector3d GetPosition() const;
	virtual vector3d GetVelocity() const;
	matrix4x4d GetTransformRelTo(const Frame *relTo) const;
	vector3d GetAngVelocity() const;
	void SetAngVelocity(vector3d v);
	void SetMesh(ObjMesh *m);
	virtual bool OnCollision(Object *o, Uint32 flags, double relVel);
	vector3d GetAngularMomentum() const;
	double GetAngularInertia() const { return m_angInertia; }
	void SetMassDistributionFromModel();
	void DisableBodyOnly() { m_enabled = false; }
	bool IsEnabled() const { return m_enabled; }
	virtual void Disable();
	virtual void Enable();
	virtual double GetMass() const { return m_mass; }
	virtual void TimeStepUpdate(const float timeStep);
	void CalcExternalForce();
	void ApplyAccel(const float timeStep);
	void UndoTimestep();
	
	void SetMass(double);
	void AddForce(const vector3d);
	void AddTorque(const vector3d);
	void SetForce(const vector3d);
	void SetTorque(const vector3d);
	vector3d GetForce() const { return m_force; }
	// body-relative forces
	void AddRelForce(const vector3d);
	void AddRelTorque(const vector3d);
	vector3d GetExternalForce() const { return m_externalForce; }
	vector3d GetAtmosForce() const { return m_atmosForce; }		
	vector3d GetGravityForce() const { return m_gravityForce; }
	virtual void UpdateInterpolatedTransform(double alpha);

	virtual void PostLoadFixup(Space *space);
protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);
private:
	// new odeless turd
	matrix4x4d m_orient; // contains pos
	matrix4x4d m_oldOrient;
	vector3d m_force;
	vector3d m_torque;
	vector3d m_vel;
	vector3d m_angVel;
	vector3d m_oldAngDisplacement;
	double m_mass;
	double m_massRadius; // set in a mickey-mouse fashion from the collision mesh and used to calculate m_angInertia
	double m_angInertia; // always sphere mass distribution
	bool m_enabled;
	//
	vector3d m_externalForce;
	vector3d m_atmosForce;
	vector3d m_gravityForce;	
	// for time accel reduction fudge
	vector3d m_lastForce;
	vector3d m_lastTorque;
};

#endif /* _DYNAMICBODY_H */
