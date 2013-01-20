// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
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
	float fuel_use; // percentage (ie, 0--100) of tank used per second at full thrust
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

	void TimeAccelAdjust(const float timeStep);
	void SetDecelerating(bool decel) { m_decelerating = decel; }
	bool IsDecelerating() const { return m_decelerating; }

	virtual void NotifyRemoved(const Body* const removedBody);
	virtual bool OnCollision(Object *o, Uint32 flags, double relVel);
	virtual bool OnDamage(Object *attacker, float kgDamage);

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
	vector3d AIChangeVelDir(const vector3d &diffvel);	// world space, maintain direction
	void AIMatchAngVelObjSpace(const vector3d &angvel);
	double AIFaceUpdir(const vector3d &updir, double av=0);
	double AIFaceDirection(const vector3d &dir, double av=0);
	vector3d AIGetLeadDir(const Body *target, const vector3d& targaccel, int gunindex=0);
	double AITravelTime(const vector3d &reldir, double targdist, const vector3d &relvel, double endspeed, double maxdecel);

	// old stuff, deprecated
	void AIAccelToModelRelativeVelocity(const vector3d v);
	void AIModelCoordsMatchAngVel(vector3d desiredAngVel, double softness);
	void AIModelCoordsMatchSpeedRelTo(const vector3d v, const Ship *);

	void AIClearInstructions();
	bool AIIsActive() { return m_curAICmd ? true : false; }
	void AIGetStatusText(char *str);

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
	void AIDock(SpaceStation *target);
	void AIFlyTo(Body *target);
	void AIOrbit(Body *target, double alt);
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

	// fuel left, 0.0-1.0
	double GetFuel() const { return m_thrusterFuel;	}
	void SetFuel(const double f) { m_thrusterFuel = Clamp(f, 0.0, 1.0); }
	double GetFuelReserve() const { return m_reserveFuel; }
	void SetFuelReserve(const double f) { m_reserveFuel = Clamp(f, 0.0, 1.0); }

	// percentage (ie, 0--100) of tank used per second at full thrust
	double GetFuelUseRate() const;
	double GetSpeedReachedWithFuel() const;

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
	void UpdateFuel(float timeStep, const vector3d &thrust);
	void OnEquipmentChange(Equip::Type e);
	void EnterHyperspace();

	shipstats_t m_stats;
	ShipType *m_type;

	FlightState m_flightState;
	bool m_testLanded;
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
	bool m_decelerating;

	double m_thrusterFuel; 	// remaining fuel 0.0-1.0
	double m_reserveFuel;	// 0-1, fuel not to touch for the current AI program

	int m_dockedWithIndex; // deserialisation

	SceneGraph::Animation *m_landingGearAnimation;
};



#endif /* _SHIP_H */


