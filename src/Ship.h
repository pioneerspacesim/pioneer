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
	Ship(ShipType::Type shipType);
	virtual Object::Type GetType() { return Object::SHIP; }
	virtual void SetDockedWith(SpaceStation *);
	SpaceStation *GetDockedWith() { return m_dockedWith; }
	void SetNavTarget(Body* const target) { m_navTarget = target; }
	Body *GetNavTarget() const { return m_navTarget; }
	void SetCombatTarget(Body* const target) { m_combatTarget = target; }
	Body *GetCombatTarget() const { return m_combatTarget; }
	virtual void Render(const Frame *camFrame);
	void SetThrusterState(enum ShipType::Thruster t, float level);
	void SetAngThrusterState(int axis, float level) { m_angThrusters[axis] = CLAMP(level, -1, 1); }
	void ClearThrusterState();
	void SetGunState(int idx, int state);
	const ShipType &GetShipType();
	void CalcStats(shipstats_t *stats);
	void UpdateMass();
	void SetWheelState(bool down);
	virtual void TimeStepUpdate(const float timeStep);
	virtual void NotifyDeath(const Body* const dyingBody);
	
	class LaserObj: public Object {
	public:
		virtual Object::Type GetType() { return Object::LASER; }
		Ship *owner;
	};

	EquipSet m_equipment;

protected:
	void RenderLaserfire();

	SpaceStation *m_dockedWith;
	enum ShipType::Type m_shipType;
	Uint32 m_gunState[ShipType::GUNMOUNT_MAX];
private:
	float m_wheelState;
	float m_wheelTransition;

	float m_thrusters[ShipType::THRUSTER_MAX];
	float m_angThrusters[3];
	dGeomID m_tempLaserGeom[ShipType::GUNMOUNT_MAX];

	LaserObj m_laserCollisionObj;
	Body* m_navTarget;
	Body* m_combatTarget;
};

#endif /* _SHIP_H */
