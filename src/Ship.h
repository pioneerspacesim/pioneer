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
class AICommand;

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

	void SetThrusterState(int axis, double level) { m_thrusters[axis] = Clamp(level, -1.0, 1.0); }
	void SetThrusterState(const vector3d &levels);
	vector3d GetThrusterState() const { return m_thrusters; }
	void SetAngThrusterState(int axis, double level) { m_angThrusters[axis] = Clamp(level, -1.0, 1.0); }
	void SetAngThrusterState(const vector3d &levels);
	vector3d GetAngThrusterState() const { return m_angThrusters; }
	void ClearThrusterState();

	vector3d GetMaxThrust(const vector3d &dir);
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
	void ClearHyperspaceTarget();
	void TryHyperspaceTo(const SBodyPath *dest);
	enum HyperjumpStatus {
		HYPERJUMP_OK,
		HYPERJUMP_CURRENT_SYSTEM,
		HYPERJUMP_NO_DRIVE,
		HYPERJUMP_OUT_OF_RANGE,
		HYPERJUMP_INSUFFICIENT_FUEL
	};
	bool CanHyperspaceTo(const SBodyPath *dest, int &outFuelRequired, double &outDurationSecs, enum HyperjumpStatus *outStatus = 0);
	void UseHyperspaceFuel(const SBodyPath *dest);
	float GetHyperspaceCountdown() const { return m_hyperspace.countdown; }
	Equip::Type GetHyperdriveFuelType() const;
	float GetWeakestThrustersForce() const;
	// 0 to 1.0 is alive, > 1.0 = death
	double GetHullTemperature() const;
	void UseECM();
	virtual bool FireMissile(int idx, Ship *target);

	enum AlertState {
		ALERT_NONE,
		ALERT_SHIP_NEARBY,
		ALERT_SHIP_FIRING,
	};
	AlertState GetAlertState() { return m_alertState; }

	bool AIMatchVel(const vector3d &vel);
	bool AIChangeVelBy(const vector3d &diffvel);		// acts in obj space
	double AIMatchPosVel(const vector3d &targpos, const vector3d &curvel, double targvel, const vector3d &maxthrust);
	void AIMatchAngVelObjSpace(const vector3d &angvel);
	void AIFaceDirectionImmediate(const vector3d &dir);
	double AIFaceOrient(const vector3d &dir, const vector3d &updir);
	double AIFaceDirection(const vector3d &dir, double av=0);
	vector3d AIGetNextFramePos();
	vector3d AIGetLeadDir(const Body *target, const vector3d& targaccel, int gunindex=0);

	// old stuff, deprecated
	void AIAccelToModelRelativeVelocity(const vector3d v);
	void AIModelCoordsMatchAngVel(vector3d desiredAngVel, double softness);
	void AIModelCoordsMatchSpeedRelTo(const vector3d v, const Ship *);

	void AIClearInstructions();
	bool AIIsActive() { return m_curAICmd ? true : false; }
	enum AIError { NONE=0, GRAV_TOO_HIGH, REFUSED_PERM };
	AIError AIMessage(AIError msg=NONE) { AIError tmp = m_aiMessage; m_aiMessage = msg; return tmp; }

	void AIKamikaze(Body *target);
	void AIKill(Ship *target);
	void AIJourney(SBodyPath &dest);
	void AIDock(SpaceStation *target);
	void AIFlyTo(Body *target);
	void AIOrbit(Body *target, double alt);
	void AIHoldPosition();

	void AIBodyDeleted(const Body* const body) {};		// todo: signals

	EquipSet m_equipment;			// shouldn't be public?...

	virtual void PostLoadFixup();
	/* MarketAgent stuff */
	int GetStock(Equip::Type t) const { assert(0); return 0; }
	bool CanBuy(Equip::Type t, bool verbose) const;
	bool CanSell(Equip::Type t, bool verbose) const;
	bool DoesSell(Equip::Type t) const { return true; }
	Sint64 GetPrice(Equip::Type t) const;

	const ShipFlavour *GetFlavour() const { return &m_shipFlavour; }
	// used to change ship label or colour. asserts if you try to change type
	void UpdateFlavour(const ShipFlavour *f);
	// used when buying a new ship. changes the flavour and resets cargo,
	// equipment, etc
	void ResetFlavour(const ShipFlavour *f);

	float GetPercentShields() const;
	float GetPercentHull() const;
	void SetPercentHull(float);
	float GetGunTemperature(int idx) const { return m_gunTemperature[idx]; }
	
	sigc::signal<void> onDock;				// JJ: check what these are for
	sigc::signal<void> onUndock;
protected:
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);
	void RenderLaserfire();

	bool AITimeStep(float timeStep);		// returns true if complete

	virtual void SetAlertState(AlertState as) { m_alertState = as; }

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
	void Init();
	bool IsFiringLasers();
	void TestLanded();
	void UpdateAlertState();

	FlightState m_flightState;
	bool m_testLanded;
	float m_launchLockTimeout;
	float m_wheelState;
	float m_wheelTransition;

	vector3d m_thrusters;
	vector3d m_angThrusters;
	Body* m_navTarget;
	Body* m_combatTarget;
	shipstats_t m_stats;

	AlertState m_alertState;
	float m_lastFiringAlert;

	struct HyperspacingOut {
		int followHypercloudId;
		SBodyPath dest;
		// > 0 means active
		float countdown;
	} m_hyperspace;

	AICommand *m_curAICmd;
	AIError m_aiMessage;
};



#endif /* _SHIP_H */


