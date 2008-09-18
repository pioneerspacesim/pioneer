#ifndef _SHIP_H
#define _SHIP_H

#include "libs.h"
#include "DynamicBody.h"
#include "ShipType.h"
#include "sbre/sbre.h"

class SpaceStation;

struct shipstats_t {
	int max_capacity;
	int used_capacity;
	int free_capacity;
	int total_mass; // cargo, equipment + hull
	float hyperspace_range;
};

class Ship: public DynamicBody {
public:
	OBJDEF(Ship, DynamicBody, SHIP);
	Ship(ShipType::Type shipType);
	virtual void SetDockedWith(SpaceStation *, int port);
	SpaceStation *GetDockedWith() { return m_dockedWith; }
	void SetNavTarget(Body* const target);
	Body *GetNavTarget() const { return m_navTarget; }
	void SetCombatTarget(Body* const target);
	Body *GetCombatTarget() const { return m_combatTarget; }
	virtual void Render(const Frame *camFrame);
	void SetThrusterState(enum ShipType::Thruster t, float level);
	void SetAngThrusterState(int axis, float level) { m_angThrusters[axis] = CLAMP(level, -1, 1); }
	void ClearThrusterState();
	void SetGunState(int idx, int state);
	const ShipType &GetShipType();
	void CalcStats(shipstats_t *stats);
	void UpdateMass();
	vector3d CalcRotDamping();
	bool SetWheelState(bool down); // returns success of state change, NOT state itself
	void Blastoff();
	float GetDockingTimer() { return m_dockingTimer; }
	void SetDockingTimer(float t) { m_dockingTimer = t; }
	virtual void TimeStepUpdate(const float timeStep);
	virtual void NotifyDeath(const Body* const dyingBody);
	virtual bool OnCollision(Body *b, Uint32 flags);
	enum FlightState { FLYING, LANDED };
       	FlightState GetFlightState() const { return m_flightState; }
	float GetWheelState() const { return m_wheelState; }
	
	class LaserObj: public Object {
	public:
		OBJDEF(LaserObj, Object, LASER);
		Ship *owner;
	};

	EquipSet m_equipment;

protected:
	void RenderLaserfire();

	SpaceStation *m_dockedWith;
	int m_dockedWithPort;
	enum ShipType::Type m_shipType;
	Uint32 m_gunState[ShipType::GUNMOUNT_MAX];
private:
	bool IsFiringLasers();
	void TestLanded();

	FlightState m_flightState;
	bool m_testLanded;
	float m_launchLockTimeout;
	float m_wheelState;
	float m_wheelTransition;

	float m_thrusters[ShipType::THRUSTER_MAX];
	float m_angThrusters[3];
	float m_dockingTimer;
	dGeomID m_tempLaserGeom[ShipType::GUNMOUNT_MAX];

	LaserObj m_laserCollisionObj;
	Body* m_navTarget;
	Body* m_combatTarget;
};

#endif /* _SHIP_H */
