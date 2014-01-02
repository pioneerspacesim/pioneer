// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _DYNAMICBODY_H
#define _DYNAMICBODY_H

#include "Body.h"
#include "ModelBody.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "Orbit.h"

class DynamicBody: public ModelBody {
public:
	OBJDEF(DynamicBody, ModelBody, DYNAMICBODY);
	DynamicBody();
	virtual ~DynamicBody();
	virtual vector3d GetVelocity() const;
	virtual void SetVelocity(const vector3d &v);
	virtual void SetFrame(Frame *f);
	vector3d GetAngVelocity() const;
	void SetAngVelocity(const vector3d &v);
	void SetMesh(ObjMesh *m);
	virtual bool OnCollision(Object *o, Uint32 flags, double relVel);
	vector3d GetAngularMomentum() const;
	double GetAngularInertia() const { return m_angInertia; }
	void SetMassDistributionFromModel();
	void SetMoving(bool isMoving) { m_isMoving = isMoving; }
	bool IsMoving() const { return m_isMoving; }
	virtual double GetMass() const { return m_mass; }	// XXX don't override this
	virtual void TimeStepUpdate(const float timeStep);
	void CalcExternalForce();
	void UndoTimestep();

	void SetMass(double);
	void AddForce(const vector3d &);
	void AddTorque(const vector3d &);
	void SetForce(const vector3d &);
	void SetTorque(const vector3d &);
	vector3d GetLastForce() const { return m_lastForce; }
	vector3d GetLastTorque() const { return m_lastTorque; }
	// body-relative forces
	void AddRelForce(const vector3d &);
	void AddRelTorque(const vector3d &);
	vector3d GetExternalForce() const { return m_externalForce; }
	vector3d GetAtmosForce() const { return m_atmosForce; }
	vector3d GetGravityForce() const { return m_gravityForce; }
	virtual void UpdateInterpTransform(double alpha);

	virtual void PostLoadFixup(Space *space);

	Orbit ComputeOrbit() const;
protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);
private:
	vector3d m_oldPos;
	vector3d m_oldAngDisplacement;

	vector3d m_force;
	vector3d m_torque;
	vector3d m_vel;
	vector3d m_angVel;
	double m_mass;
	double m_massRadius; // set in a mickey-mouse fashion from the collision mesh and used to calculate m_angInertia
	double m_angInertia; // always sphere mass distribution
	bool m_isMoving;

	vector3d m_externalForce;
	vector3d m_atmosForce;
	vector3d m_gravityForce;
	// for time accel reduction fudge
	vector3d m_lastForce;
	vector3d m_lastTorque;
};

#endif /* _DYNAMICBODY_H */
