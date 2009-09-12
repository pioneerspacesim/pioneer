#ifndef _SHIP_H
#define _SHIP_H

#include "libs.h"
#include "DynamicBody.h"
#include "ShipType.h"
#include "sbre/sbre.h"
#include "MarketAgent.h"
#include "ShipFlavour.h"
#include <list>

class SpaceStation;
struct SBodyPath;

struct shipstats_t {
	int max_capacity;
	int used_capacity;
	int used_cargo;
	int free_capacity;
	int total_mass; // cargo, equipment + hull
	float hull_mass_left; // effectively hitpoints
	float hyperspace_range;
	float hyperspace_range_max;
};

class Ship: public DynamicBody, public MarketAgent {
public:
	OBJDEF(Ship, DynamicBody, SHIP);
	Ship(ShipType::Type shipType);
	Ship() {}
	virtual void SetDockedWith(SpaceStation *, int port);
	/** Use GetDockedWith() to determine if docked */
	SpaceStation *GetDockedWith() { return m_dockedWith; }
	int GetDockingPort() const { return m_dockedWithPort; }
	void SetNavTarget(Body* const target);
	Body *GetNavTarget() const { return m_navTarget; }
	void SetCombatTarget(Body* const target);
	Body *GetCombatTarget() const { return m_combatTarget; }
	virtual void Render(const Frame *camFrame);
	void SetThrusterState(enum ShipType::Thruster t, float level);
	void SetAngThrusterState(int axis, float level) { m_angThrusters[axis] = CLAMP(level, -1, 1); }
	vector3f GetAngThrusterState() const { return vector3f(m_angThrusters); }
	void ClearThrusterState();
	void SetGunState(int idx, int state);
	const ShipType &GetShipType();
	const shipstats_t *CalcStats();
	void UpdateMass();
	vector3d CalcRotDamping();
	bool SetWheelState(bool down); // returns success of state change, NOT state itself
	void Blastoff();
	virtual void TimeStepUpdate(const float timeStep);
	virtual void StaticUpdate(const float timeStep);
	virtual void NotifyDeath(const Body* const dyingBody);
	virtual bool OnCollision(Body *b, Uint32 flags, double relVel);
	virtual bool OnDamage(Body *attacker, float kgDamage);
	enum FlightState { FLYING, LANDED, DOCKING };
       	FlightState GetFlightState() const { return m_flightState; }
	void SetFlightState(FlightState s) { m_flightState = s; }
	float GetWheelState() const { return m_wheelState; }
	bool Jettison(Equip::Type t);
	bool CanHyperspaceTo(const SBodyPath *dest, int &fuelRequired);
	void UseHyperspaceFuel(const SBodyPath *dest);
	void AIFaceDirection(const vector3d &dir);
	void AISlowFaceDirection(const vector3d &dir);
	void AIAccelToModelRelativeVelocity(const vector3d v);
	void AIModelCoordsMatchSpeedRelTo(const vector3d v, const Ship *);
	
	EquipSet m_equipment;

	enum AICommand { DO_NOTHING, DO_KILL, DO_FLY_TO };
	void AIInstruct(enum AICommand, void *arg);
	void AIClearInstructions() { m_todo.clear(); }
	virtual void PostLoadFixup();
	/* MarketAgent stuff */
	bool CanBuy(Equip::Type t) const;
	bool CanSell(Equip::Type t) const;
	int GetPrice(Equip::Type t) const;
	void ChangeFlavour(const ShipFlavour *f);
	const ShipFlavour *GetFlavour() const { return &m_shipFlavour; }
	
	sigc::signal<void> onDock;
	sigc::signal<void> onUndock;
protected:
	virtual void Save();
	virtual void Load();
	void RenderLaserfire();

	SpaceStation *m_dockedWith;
	int m_dockedWithPort;
	ShipFlavour m_shipFlavour;
	Uint32 m_gunState[ShipType::GUNMOUNT_MAX];
	float m_gunRecharge[ShipType::GUNMOUNT_MAX];
	/* MarketAgent stuff */
	void Bought(Equip::Type t);
	void Sold(Equip::Type t);
private:
	void FireWeapon(int num);
	void AITimeStep(const float timeStep);
	void Init();
	bool IsFiringLasers();
	void TestLanded();

	FlightState m_flightState;
	bool m_testLanded;
	float m_launchLockTimeout;
	float m_wheelState;
	float m_wheelTransition;

	float m_thrusters[ShipType::THRUSTER_MAX];
	float m_angThrusters[3];
	Body* m_navTarget;
	Body* m_combatTarget;
	shipstats_t m_stats;
	class AIInstruction {
	public:
		AICommand cmd;
		void *arg;
		AIInstruction(AICommand c, void *a): cmd(c), arg(a) {}
	};
	std::list<AIInstruction> m_todo;
	void AIBodyHasDied(const Body* const body);
	bool AICmdKill(const Ship *);
	bool AICmdFlyTo(const Body *);
};

#endif /* _SHIP_H */
