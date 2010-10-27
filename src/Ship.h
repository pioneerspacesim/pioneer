#ifndef _SHIP_H
#define _SHIP_H

#include "libs.h"
#include "DynamicBody.h"
#include "ShipType.h"
#include "MarketAgent.h"
#include "ShipFlavour.h"
// only for SBodyPath
#include "StarSystem.h"
#include "BezierCurve.h"
#include <list>

class SpaceStation;
class HyperspaceCloud;

struct shipstats_t {
	int max_capacity;
	int used_capacity;
	int used_cargo;
	int free_capacity;
	int total_mass; // cargo, equipment + hull
	float hull_mass_left; // effectively hitpoints
	float hyperspace_range;
	float hyperspace_range_max;
	float shield_mass;
	float shield_mass_left;
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
	virtual void Render(const vector3d &viewCoords, const matrix4x4d &viewTransform);
	void SetThrusterState(enum ShipType::Thruster t, float level);
	float GetThrusterState(enum ShipType::Thruster t) const { return m_thrusters[t]; }
	void SetAngThrusterState(int axis, float level) { m_angThrusters[axis] = CLAMP(level, -1, 1); }
	vector3f GetAngThrusterState() const { return vector3f(m_angThrusters); }
	void ClearThrusterState();
	void SetGunState(int idx, int state);
	const ShipType &GetShipType() const;
	const shipstats_t *CalcStats();
	void UpdateMass();
	virtual bool SetWheelState(bool down); // returns success of state change, NOT state itself
	void Blastoff();
	bool Undock();
	virtual void TimeStepUpdate(const float timeStep);
	virtual void StaticUpdate(const float timeStep);
	virtual void NotifyDeleted(const Body* const deletedBody);
	virtual bool OnCollision(Object *o, Uint32 flags, double relVel);
	virtual bool OnDamage(Object *attacker, float kgDamage);
	enum FlightState { FLYING, LANDED, DOCKING };
       	FlightState GetFlightState() const { return m_flightState; }
	void SetFlightState(FlightState s) { m_flightState = s; }
	float GetWheelState() const { return m_wheelState; }
	bool Jettison(Equip::Type t);
	const SBodyPath *GetHyperspaceTarget() const { return &m_hyperspace.dest; }
	int GetHyperspaceCloudTargetId() { return m_hyperspace.followHypercloudId; }
	// follow departure cloud
	void SetHyperspaceTarget(HyperspaceCloud *cloud);
	// just jump to near an SBody
	void SetHyperspaceTarget(const SBodyPath *path);
	void TryHyperspaceTo(const SBodyPath *dest);
	bool CanHyperspaceTo(const SBodyPath *dest, int &outFuelRequired, double &outDurationSecs);
	void UseHyperspaceFuel(const SBodyPath *dest);
	float GetHyperspaceCountdown() const { return m_hyperspace.countdown; }
	Equip::Type GetHyperdriveFuelType() const;
	float GetWeakestThrustersForce() const;
	// 0 to 1.0 is alive, > 1.0 = death
	float GetHullTemperature() const;
	void UseECM();
	void AIFaceDirection(const vector3d &dir);
	void AISlowOrient(const matrix4x4d &dir);
	void AISlowFaceDirection(const vector3d &dir);
	void AIAccelToModelRelativeVelocity(const vector3d v);
	void AIModelCoordsMatchAngVel(vector3d desiredAngVel, float softness);
	void AIModelCoordsMatchSpeedRelTo(const vector3d v, const Ship *);
	
	EquipSet m_equipment;

	enum AICommand { DO_NOTHING, DO_KILL, DO_FLY_TO, DO_KAMIKAZE, DO_LOW_ORBIT, DO_MEDIUM_ORBIT, DO_HIGH_ORBIT, DO_DOCK, DO_FOLLOW_PATH, DO_JOURNEY };
	void AIInstruct(enum AICommand, void *arg);
	void AIInstructJourney(const SBodyPath &path);
	void AIClearInstructions() { m_todo.clear(); }
	virtual void PostLoadFixup();
	/* MarketAgent stuff */
	int GetStock(Equip::Type t) const { assert(0); return 0; }
	bool CanBuy(Equip::Type t, bool verbose) const;
	bool CanSell(Equip::Type t, bool verbose) const;
	bool DoesSell(Equip::Type t) const { return true; }
	Sint64 GetPrice(Equip::Type t) const;
	void ChangeFlavour(const ShipFlavour *f);
	const ShipFlavour *GetFlavour() const { return &m_shipFlavour; }
	float GetPercentShields() const;
	float GetPercentHull() const;
	void SetPercentHull(float);
	float GetGunTemperature(int idx) const { return m_gunTemperature[idx]; }
	
	sigc::signal<void> onDock;
	sigc::signal<void> onUndock;
protected:
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);
	void RenderLaserfire();

	SpaceStation *m_dockedWith;
	int m_dockedWithPort;
	ShipFlavour m_shipFlavour;
	Uint32 m_gunState[ShipType::GUNMOUNT_MAX];
	float m_gunRecharge[ShipType::GUNMOUNT_MAX];
	float m_gunTemperature[ShipType::GUNMOUNT_MAX];
	float m_ecmRecharge;
	/* MarketAgent stuff */
	void Bought(Equip::Type t);
	void Sold(Equip::Type t);
private:
	float GetECMRechargeTime();
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

	struct HyperspacingOut {
		int followHypercloudId;
		SBodyPath dest;
		// > 0 means active
		float countdown;
	} m_hyperspace;

	class AIInstruction {
	public:
		AICommand cmd;
		Body *target;
		BezierCurve path;
		double endTime;
		double startTime;
		Frame *frame;
		SBodyPath journeyDest;

		AIInstruction(AICommand c): cmd(c), path(0) {
			target = 0;
			endTime = 0;
			startTime = 0;
			frame = 0;
		}
	};
	std::list<AIInstruction> m_todo;
	bool AIAddAvoidancePathOnWayTo(const Body *target);
	bool AIArePlanetsInTheWayOfGettingTo(const vector3d &target, Body **obstructor, double &outDist);
	AIInstruction &AIPrependInstruction(enum AICommand cmd, void *arg);
	void AIBodyDeleted(const Body* const body);
	bool AICmdJourney(AIInstruction &);
	bool AICmdDock(AIInstruction &, SpaceStation *);
	bool AICmdKill(const Ship *);
	bool AICmdOrbit(AIInstruction &, double orbitHeight);
	bool AICmdKamikaze(const Ship *);
	bool AICmdFlyTo(AIInstruction &);
	void AITrySetBodyRelativeThrust(const vector3d &force);
	bool AIFollowPath(AIInstruction &, Frame *f, bool pointShipAtVelocityVector = false);
};

#endif /* _SHIP_H */
