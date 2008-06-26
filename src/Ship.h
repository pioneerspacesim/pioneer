#ifndef _SHIP_H
#define _SHIP_H

#include "libs.h"
#include "RigidBody.h"
#include "ShipType.h"

class SpaceStation;

class Ship: public RigidBody {
public:
	Ship(ShipType::Type shipType);
	virtual void AITurn();
	virtual Object::Type GetType() { return Object::SHIP; }
	virtual void SetDockedWith(SpaceStation *);
	SpaceStation *GetDockedWith() { return m_dockedWith; }
	virtual void Render(const Frame *camFrame);
	void SetMesh(ObjMesh *m);
	void SetThrusterState(enum ShipType::Thruster t, float level);
	void SetAngThrusterState(int axis, float level) { m_angThrusters[axis] = CLAMP(level, -1, 1); }
	void ClearThrusterState();
	void SetGunState(int idx, int state);
	
	class LaserObj: public Object {
	public:
		virtual Object::Type GetType() { return Object::LASER; }
		Ship *owner;
	};
protected:
	void RenderLaserfire();

	const ShipType &GetShipType();
	SpaceStation *m_dockedWith;
	enum ShipType::Type m_shipType;
	ObjMesh *m_mesh;
	Uint32 m_gunState[ShipType::GUNMOUNT_MAX];
private:
	float m_thrusters[ShipType::THRUSTER_MAX];
	float m_angThrusters[3];
	dGeomID m_tempLaserGeom[ShipType::GUNMOUNT_MAX];

	LaserObj m_laserCollisionObj;
};

#endif /* _SHIP_H */
