// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIP_H
#define _SHIP_H

#include "libs.h"
#include "DynamicBody.h"
#include "ShipType.h"
#include "EquipSet.h"
#include "ShipFlavour.h"
#include "galaxy/SystemPath.h"
#include "BezierCurve.h"
#include "Serializer.h"
#include "Camera.h"
#include "scenegraph/SceneGraph.h"
#include <list>

class SpaceStation;
class HyperspaceCloud;
class AICommand;
class ShipController;
class CargoBody;
namespace Graphics { class Renderer; }

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
	float fuel_tank_mass; //thruster, not hyperspace fuel
	float fuel_tank_mass_left;
	float fuel_use;
};

class SerializableEquipSet: public EquipSet {
public:
	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);
};

class Ship: public DynamicBody {
	friend class ShipController; //only controllers need access to AITimeStep
	friend class PlayerShipController;
public:
	enum Animation { // <enum scope='Ship' name=ShipAnimation prefix=ANIM_>
		ANIM_WHEEL_STATE
	};

	OBJDEF(Ship, DynamicBody, SHIP);
	Ship(ShipType::Id shipId);
	Ship() {} //default constructor used before Load
	virtual ~Ship();
	void SetController(ShipController *c); //deletes existing
	ShipController *GetController() const { return m_controller; }
	virtual bool IsPlayerShip() const { return false; } //XXX to be replaced with an owner check

	virtual void SetDockedWith(SpaceStation *, int port);
	/** Use GetDockedWith() to determine if docked */
	SpaceStation *GetDockedWith() const { return m_dockedWith; }
	int GetDockingPort() const { return m_dockedWithPort; }
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform);

	void SetThrusterState(int axis, double level) {
		if (m_thrusterFuel <= 0.f) level = 0.0;
		m_thrusters[axis] = Clamp(level, -1.0, 1.0);
	}
	void SetThrusterState(const vector3d &levels);
	vector3d GetThrusterState() const { return m_thrusters; }
	void SetAngThrusterState(int axis, double level) { m_angThrusters[axis] = Clamp(level, -1.0, 1.0); }
	void SetAngThrusterState(const vector3d &levels);
	vector3d GetAngThrusterState() const { return m_angThrusters; }
	void ClearThrusterState();

	vector3d GetMaxThrust(const vector3d &dir) const;
	double GetAccelFwd() const { return -GetShipType().linThrust[ShipType::THRUSTER_FORWARD] / GetMass(); }
	double GetAccelRev() const { return GetShipType().linThrust[ShipType::THRUSTER_REVERSE] / GetMass(); }
	double GetAccelUp() const { return GetShipType().linThrust[ShipType::THRUSTER_UP] / GetMass(); }
	double GetAccelMin() const;

	const ShipType &GetShipType() const { return *m_type; }
	void UpdateEquipStats();
	void UpdateFuelStats();
	void UpdateStats();
	const shipstats_t &GetStats() const { return m_stats; }

	void Explode();
	void SetGunState(int idx, int state);
	void UpdateMass();
	virtual bool SetWheelState(bool down); // returns success of state change, NOT state itself
	void Blastoff();
	bool Undock();
	virtual void TimeStepUpdate(const float timeStep);
	virtual void StaticUpdate(const float timeStep);
	void ApplyAccel(const float timeStep);

	virtual void NotifyRemoved(const Body* const removedBody);
	virtual bool OnCollision(Object *o, Uint32 flags, double relVel);
	virtual bool OnDamage(Object *attacker, float kgDamage);

	void SetManualRotationState(bool rotationState) { m_manualRotation = rotationState; }
	bool GetManualRotationState() { return m_manualRotation; }

	enum FlightState { // <enum scope='Ship' name=ShipFlightState>
		FLYING,     // open flight (includes autopilot)
		DOCKING,    // in docking animation
		DOCKED,     // docked with station
		LANDED,     // rough landed (not docked)
		HYPERSPACE, // in hyperspace
	};

	FlightState GetFlightState() const { return m_flightState; }
	void SetFlightState(FlightState s);
	float GetWheelState() const { return m_wheelState; }
	bool SpawnCargo(CargoBody * c_body) const;

	virtual bool IsInSpace() const { return (m_flightState != HYPERSPACE); }

	void SetHyperspaceDest(const SystemPath &dest) { m_hyperspace.dest = dest; }
	const SystemPath &GetHyperspaceDest() const { return m_hyperspace.dest; }
	double GetHyperspaceDuration() const { return m_hyperspace.duration; }

	enum HyperjumpStatus { // <enum scope='Ship' name=ShipJumpStatus prefix=HYPERJUMP_>
		HYPERJUMP_OK,
		HYPERJUMP_CURRENT_SYSTEM,
		HYPERJUMP_NO_DRIVE,
		HYPERJUMP_DRIVE_ACTIVE,
		HYPERJUMP_OUT_OF_RANGE,
		HYPERJUMP_INSUFFICIENT_FUEL,
		HYPERJUMP_SAFETY_LOCKOUT
	};

	HyperjumpStatus GetHyperspaceDetails(const SystemPath &dest, int &outFuelRequired, double &outDurationSecs);
	HyperjumpStatus CheckHyperspaceTo(const SystemPath &dest, int &outFuelRequired, double &outDurationSecs);
	HyperjumpStatus CheckHyperspaceTo(const SystemPath &dest) {
		int unusedFuel;
		double unusedDuration;
		return CheckHyperspaceTo(dest, unusedFuel, unusedDuration);
	}
	bool CanHyperspaceTo(const SystemPath &dest, HyperjumpStatus &status) {
		status = CheckHyperspaceTo(dest);
		return (status == HYPERJUMP_OK);
	}
	bool CanHyperspaceTo(const SystemPath &dest) { return (CheckHyperspaceTo(dest) == HYPERJUMP_OK); }

	virtual Ship::HyperjumpStatus StartHyperspaceCountdown(const SystemPath &dest);
	float GetHyperspaceCountdown() const { return m_hyperspace.countdown; }
	bool IsHyperspaceActive() const { return (m_hyperspace.countdown > 0.0); }
	virtual void ResetHyperspaceCountdown();

	Equip::Type GetHyperdriveFuelType() const;
	// 0 to 1.0 is alive, > 1.0 = death
	double GetHullTemperature() const;
	void UseECM();
	virtual bool FireMissile(int idx, Ship *target);

	enum AlertState { // <enum scope='Ship' name=ShipAlertStatus prefix=ALERT_>
		ALERT_NONE,
		ALERT_SHIP_NEARBY,
		ALERT_SHIP_FIRING,
	};
	AlertState GetAlertState() { return m_alertState; }

	bool AIMatchVel(const vector3d &vel);
	bool AIChangeVelBy(const vector3d &diffvel);		// acts in obj space
	bool AIMatchPosVel2(const vector3d &reldir, double targdist, const vector3d &relvel, double endspeed, double maxthrust);
	double AIMatchPosVel(const vector3d &relpos, const vector3d &relvel, double endspeed, const vector3d &maxthrust);
	void AIMatchAngVelObjSpace(const vector3d &angvel);
	void AIFaceDirectionImmediate(const vector3d &dir);
	bool AIFaceOrient(const vector3d &dir, const vector3d &updir);
	double AIFaceDirection(const vector3d &dir, double av=0);
	vector3d AIGetNextFramePos();
	vector3d AIGetLeadDir(const Body *target, const vector3d& targaccel, int gunindex=0);
	double AITravelTime(const vector3d &reldir, double targdist, const vector3d &relvel, double targspeed, bool flip);

	// old stuff, deprecated
	void AIAccelToModelRelativeVelocity(const vector3d v);
	void AIModelCoordsMatchAngVel(vector3d desiredAngVel, double softness);
	void AIModelCoordsMatchSpeedRelTo(const vector3d v, const Ship *);

	void AIClearInstructions();
	bool AIIsActive() { return m_curAICmd ? true : false; }
	void AIGetStatusText(char *str);
	Frame *AIGetRiskFrame();

	enum AIError { // <enum scope='Ship' name=ShipAIError prefix=AIERROR_>
		AIERROR_NONE=0,
		AIERROR_GRAV_TOO_HIGH,
		AIERROR_REFUSED_PERM,
		AIERROR_ORBIT_IMPOSSIBLE
	};
	AIError AIMessage(AIError msg=AIERROR_NONE) { AIError tmp = m_aiMessage; m_aiMessage = msg; return tmp; }

	void AIKamikaze(Body *target);
	void AIKill(Ship *target);
	//void AIJourney(SystemBodyPath &dest);
	void AIDock(SpaceStation *target, float hungriness = 0.0f);
	void AIFlyTo(Body *target, float hungriness = 0.0f);
	void AIOrbit(Body *target, double alt, float hungriness = 0.0f);
	void AIHoldPosition();

	void AIBodyDeleted(const Body* const body) {};		// todo: signals

	SerializableEquipSet m_equipment;			// shouldn't be public?...

	virtual void PostLoadFixup(Space *space);

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

	enum FuelState { // <enum scope='Ship' name=ShipFuelStatus prefix=FUEL_>
		FUEL_OK,
		FUEL_WARNING,
		FUEL_EMPTY,
	};
	FuelState GetFuelState() { return m_thrusterFuel > 0.05f ? FUEL_OK : m_thrusterFuel > 0.0f ? FUEL_WARNING : FUEL_EMPTY; }

	//fuel left, 0.0-1.0
	float GetFuel() const {	return m_thrusterFuel;	}
	//0.0 - 1.0
	void SetFuel(const float f) {	m_thrusterFuel = Clamp(f, 0.f, 1.f); }

	double GetFuelUseRate(double effectiveExhaustVelocity);
	double GetEffectiveExhaustVelocity();
	double GetVelocityReachedWithFuelUsed(float fuelUsed);

	void EnterSystem();

	HyperspaceCloud *GetHyperspaceCloud() const { return m_hyperspaceCloud; }

	sigc::signal<void> onDock;				// JJ: check what these are for
	sigc::signal<void> onUndock;

	// mutable because asking to know when state changes is not the same as
	// actually changing state
	mutable sigc::signal<void> onFlavourChanged;

protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);
	void RenderLaserfire();

	bool AITimeStep(float timeStep); // Called by controller. Returns true if complete

	virtual void SetAlertState(AlertState as) { m_alertState = as; }

	virtual void OnEnterHyperspace();
	virtual void OnEnterSystem();

	SpaceStation *m_dockedWith;
	int m_dockedWithPort;
	ShipFlavour m_shipFlavour;
	Uint32 m_gunState[ShipType::GUNMOUNT_MAX];
	float m_gunRecharge[ShipType::GUNMOUNT_MAX];
	float m_gunTemperature[ShipType::GUNMOUNT_MAX];
	float m_ecmRecharge;

	ShipController *m_controller;

private:
	float GetECMRechargeTime();
	void DoThrusterSounds() const;
	void FireWeapon(int num);
	void Init();
	bool IsFiringLasers();
	void TestLanded();
	void UpdateAlertState();
	void UpdateFuel(float timeStep);
	void OnEquipmentChange(Equip::Type e);
	void EnterHyperspace();

	shipstats_t m_stats;
	ShipType *m_type;

	FlightState m_flightState;
	bool m_testLanded;
	bool m_manualRotation;
	float m_launchLockTimeout;
	float m_wheelState;
	int m_wheelTransition;

	vector3d m_thrusters;
	vector3d m_angThrusters;

	AlertState m_alertState;
	double m_lastFiringAlert;

	struct HyperspacingOut {
		SystemPath dest;
		// > 0 means active
		float countdown;
		bool now;
		double duration;
	} m_hyperspace;
	HyperspaceCloud *m_hyperspaceCloud;

	AICommand *m_curAICmd;
	AIError m_aiMessage;

	float m_thrusterFuel; //remaining fuel 0.0-1.0
	float m_fuelUseWeights[4]; //rear, front, lateral, up&down. Rear thrusters are usually 1.0

	int m_dockedWithIndex; // deserialisation

	SceneGraph::Animation *m_landingGearAnimation;
};



#endif /* _SHIP_H */


