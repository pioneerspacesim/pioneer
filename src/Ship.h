// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIP_H
#define _SHIP_H

#include <unordered_map>

#include "DynamicBody.h"
#include "ShipType.h"
#include "galaxy/SystemPath.h"
#include "lua/LuaRef.h"
#include "scenegraph/ModelSkin.h"
#include "sound/Sound.h"

#include "FixedGuns.h"
#include "ship/Propulsion.h"

class AICommand;
class Camera;
class CargoBody;
class SpaceStation;
class HyperspaceCloud;
class Missile;
class NavLights;
class Planet;
class Sensors;
class ShipController;
class Space;

struct CollisionContact;
struct HeatGradientParameters_t;

namespace Graphics {
	class Renderer;
}

struct shipstats_t {
	int used_capacity;
	int used_cargo;
	int free_capacity;
	int static_mass;	  // cargo, equipment + hull
	float hull_mass_left; // effectively hitpoints
	float hyperspace_range;
	float hyperspace_range_max;
	float shield_mass;
	float shield_mass_left;
	float fuel_tank_mass_left;

	// cached equipment information to avoid costly Lua lookups
	int atmo_shield_cap;
	int radar_cap;
	int fuel_scoop_cap;
	int cargo_life_support_cap;
	int hull_autorepair_cap;
};

struct HyperdriveSoundsTable {
	std::string jump_sound;
	std::string warmup_sound;
	std::string abort_sound;
};

class Ship : public DynamicBody {
	friend class ShipController; //only controllers need access to AITimeStep
	friend class PlayerShipController;

public:
	OBJDEF(Ship, DynamicBody, SHIP);
	Ship() = delete;
	Ship(const Json &jsonObj, Space *space);
	Ship(const ShipType::Id &shipId);
	virtual ~Ship();

	virtual void SetFrame(FrameId fId) override;

	void SetController(ShipController *c); //deletes existing
	ShipController *GetController() const { return m_controller; }

	virtual void SetDockedWith(SpaceStation *, int port);
	/** Use GetDockedWith() to determine if docked */
	SpaceStation *GetDockedWith() const { return m_dockedWith; }
	int GetDockingPort() const { return m_dockedWithPort; }
	bool IsDocked() const { return GetFlightState() == Ship::DOCKED; }
	bool IsLanded() const { return GetFlightState() == Ship::LANDED; }

	virtual void SetLandedOn(Planet *p, float latitude, float longitude);

	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override;

	inline void ClearThrusterState()
	{
		ClearAngThrusterState();
		ClearLinThrusterState();
	}
	void UpdateLuaStats();
	void UpdateEquipStats();
	void UpdateFuelStats();
	void UpdateGunsStats();
	const shipstats_t &GetStats() const { return m_stats; }

	void Explode();
	virtual bool DoDamage(float kgDamage); // can be overloaded in Player to add audio
	void SetGunState(int idx, int state);
	void UpdateMass();
	virtual bool SetWheelState(bool down); // returns success of state change, NOT state itself
	virtual bool ManualDocking() const { return false; }
	void Blastoff();
	bool Undock();
	virtual void TimeStepUpdate(const float timeStep) override;
	virtual void StaticUpdate(const float timeStep) override;

	void TimeAccelAdjust(const float timeStep);

	bool IsDecelerating() const { return m_decelerating; }

	virtual void NotifyRemoved(const Body *const removedBody) override;
	virtual bool OnCollision(Body *o, Uint32 flags, double relVel) override;
	virtual bool OnDamage(Body *attacker, float kgDamage, const CollisionContact &contactData) override;

	enum FlightState { // <enum scope='Ship' name=ShipFlightState public>
		FLYING,		   // open flight (includes autopilot)
		DOCKING,	   // in docking animation
		UNDOCKING,	   // in docking animation
		DOCKED,		   // docked with station
		LANDED,		   // rough landed (not docked)
		JUMPING,	   // between space and hyperspace ;)
		HYPERSPACE,	   // in hyperspace
	};

	// vector3d CalcAtmoPassiveControl() const;
	vector3d CalcAtmoTorque() const;

	FlightState GetFlightState() const { return m_flightState; }
	void SetFlightState(FlightState s);
	float GetWheelState() const { return m_wheelState; }
	int GetWheelTransition() const { return m_wheelTransition; }
	bool SpawnCargo(CargoBody *c_body) const;

	LuaRef GetEquipSet() const { return m_equipSet; }

	virtual bool IsInSpace() const override { return (m_flightState != HYPERSPACE); }

	void SetHyperspaceDest(const SystemPath &dest) { m_hyperspace.dest = dest; }
	const SystemPath &GetHyperspaceDest() const { return m_hyperspace.dest; }
	double GetHyperspaceDuration() const { return m_hyperspace.duration; }
	double GetECMRechargeRemain() const { return m_ecmRecharge; }

	enum HyperjumpStatus { // <enum scope='Ship' name=ShipJumpStatus prefix=HYPERJUMP_ public>
		HYPERJUMP_OK,
		HYPERJUMP_CURRENT_SYSTEM,
		HYPERJUMP_NO_DRIVE,
		HYPERJUMP_INITIATED,
		HYPERJUMP_DRIVE_ACTIVE,
		HYPERJUMP_OUT_OF_RANGE,
		HYPERJUMP_INSUFFICIENT_FUEL,
		HYPERJUMP_SAFETY_LOCKOUT
	};

	Ship::HyperjumpStatus CheckHyperjumpCapability() const;
	virtual Ship::HyperjumpStatus InitiateHyperjumpTo(const SystemPath &dest, int warmup_time, double duration, const HyperdriveSoundsTable &sounds, LuaRef checks);
	virtual void AbortHyperjump();
	float GetHyperspaceCountdown() const { return m_hyperspace.countdown; }
	bool IsHyperspaceActive() const { return (m_hyperspace.countdown > 0.0); }

	// 0 to 1.0 is alive, > 1.0 = death
	double GetHullTemperature() const;

	enum ECMResult {
		ECM_NOT_INSTALLED,
		ECM_ACTIVATED,
		ECM_RECHARGING,
	};

	ECMResult UseECM();

	virtual Missile *SpawnMissile(ShipType::Id missile_type, int power = -1);

	enum AlertState { // <enum scope='Ship' name=ShipAlertStatus prefix=ALERT_ public>
		ALERT_NONE,
		ALERT_SHIP_NEARBY,
		ALERT_SHIP_FIRING,
		ALERT_MISSILE_DETECTED,
	};
	AlertState GetAlertState() { return m_alertState; }

	void AIClearInstructions(); // Note: defined in Ship-AI.cpp
	bool AIIsActive() const { return m_curAICmd ? true : false; }
	void AIGetStatusText(char *str); // Note: defined in Ship-AI.cpp

	void AIKamikaze(Body *target); // Note: defined in Ship-AI.cpp
	void AIKill(Ship *target);	   // Note: defined in Ship-AI.cpp
	//void AIJourney(SystemBodyPath &dest);
	void AIDock(SpaceStation *target);		// Note: defined in Ship-AI.cpp
	void AIFlyTo(Body *target);				// Note: defined in Ship-AI.cpp
	void AIOrbit(Body *target, double alt); // Note: defined in Ship-AI.cpp
	void AIHoldPosition();					// Note: defined in Ship-AI.cpp

	void AIBodyDeleted(const Body *const body){}; // Note: defined in Ship-AI.cpp // todo: signals

	const AICommand *GetAICommand() const { return m_curAICmd; }
	bool IsAIAttacking(const Ship *target) const;

	virtual void PostLoadFixup(Space *space) override;

	const ShipType *GetShipType() const { return m_type; }
	virtual void SetShipType(const ShipType::Id &shipId);

	const SceneGraph::ModelSkin &GetSkin() const { return m_skin; }
	void SetSkin(const SceneGraph::ModelSkin &skin);

	void SetPattern(unsigned int num);

	void SetLabel(const std::string &label) override;
	void SetShipName(const std::string &shipName);

	float GetAtmosphericPressureLimit() const;
	float GetPercentShields() const;
	float GetPercentHull() const;
	void SetPercentHull(float);

	void EnterSystem();

	HyperspaceCloud *GetHyperspaceCloud() const { return m_hyperspaceCloud; }

	sigc::signal<void> onDock; // JJ: check what these are for
	sigc::signal<void> onUndock;
	sigc::signal<void> onLanded;

	// mutable because asking to know when state changes is not the same as
	// actually changing state
	mutable sigc::signal<void> onFlavourChanged;

	bool IsInvulnerable() const { return m_invulnerable; }
	void SetInvulnerable(bool b) { m_invulnerable = b; }

	Sensors *GetSensors() const { return m_sensors.get(); }

	Uint8 GetRelations(Body *other) const; //0=hostile, 50=neutral, 100=ally
	void SetRelations(Body *other, Uint8 percent);

	double GetLandingPosOffset() const { return m_landingMinOffset; }

	Propulsion *GetPropulsion() { return m_propulsion; }

protected:
	vector3d CalcAtmosphericForce() const override;

	virtual void SaveToJson(Json &jsonObj, Space *space) override;

	bool AITimeStep(float timeStep); // Called by controller. Returns true if complete

	virtual void SetAlertState(AlertState as);

	virtual void OnEnterHyperspace();
	virtual void OnEnterSystem();

	SpaceStation *m_dockedWith;
	int m_dockedWithPort;

	float m_ecmRecharge;

	ShipController *m_controller;

	struct HyperspacingOut {
		SystemPath dest;
		// > 0 means active
		float countdown;
		bool now;
		double duration;
		LuaRef checks; // A Lua function to check all the conditions before the jump
		HyperdriveSoundsTable sounds;
	} m_hyperspace;

	LuaRef m_equipSet;

	Propulsion *m_propulsion;
	FixedGuns *m_fixedGuns;

private:
	float GetECMRechargeTime();
	void DoThrusterSounds() const;
	void Init();
	void TestLanded();
	void UpdateAlertState();
	void UpdateFuel(float timeStep);
	void SetShipId(const ShipType::Id &shipId);
	void EnterHyperspace();
	void InitMaterials();
	void InitEquipSet();

	bool m_invulnerable;

	static const double DEFAULT_LIFT_TO_DRAG_RATIO;

	static const float DEFAULT_SHIELD_COOLDOWN_TIME;
	float m_shieldCooldown;
	shipstats_t m_stats;
	const ShipType *m_type;
	SceneGraph::ModelSkin m_skin;

	Sound::Event m_beamLaser[2];

	FlightState m_flightState;
	bool m_testLanded;
	bool m_forceWheelUpdate;
	float m_launchLockTimeout;
	float m_wheelState;
	int m_wheelTransition;

	AlertState m_alertState;
	double m_lastAlertUpdate;
	double m_lastFiringAlert;
	bool m_shipNear;
	bool m_shipFiring;
	bool m_missileDetected;

	HyperspaceCloud *m_hyperspaceCloud;

	AICommand *m_curAICmd;

	double m_landingMinOffset; // offset from the centre of the ship used during docking

	int m_dockedWithIndex; // deserialisation

	SceneGraph::Animation *m_landingGearAnimation;
	std::unique_ptr<NavLights> m_navLights;

	std::unique_ptr<Sensors> m_sensors;
	std::unordered_map<Body *, Uint8> m_relationsMap;

	std::string m_shipName;

	double m_hydrogenScoopedAccumulator = 0;

public:
	// FIXME: these methods are deprecated; all calls should use the propulsion object directly.
	void ClearAngThrusterState() { m_propulsion->ClearAngThrusterState(); }
	void ClearLinThrusterState() { m_propulsion->ClearLinThrusterState(); }
	double GetAccelFwd() { return m_propulsion->GetAccelFwd(); }
	void SetAngThrusterState(const vector3d &levels) { m_propulsion->SetAngThrusterState(levels); }
	double GetFuel() const { return m_propulsion->GetFuel(); }
	double GetAccel(Thruster thruster) const { return m_propulsion->GetAccel(thruster); }
	void SetFuel(const double f) { m_propulsion->SetFuel(f); }
	void SetFuelReserve(const double f) { m_propulsion->SetFuelReserve(f); }

	bool AIMatchVel(const vector3d &vel, const vector3d &powerLimit = vector3d(1.0)) { return m_propulsion->AIMatchVel(vel, powerLimit); }
	double AIFaceDirection(const vector3d &dir, double av = 0) { return m_propulsion->AIFaceDirection(dir, av); }
	void SetThrusterState(int axis, double level) { return m_propulsion->SetLinThrusterState(axis, level); }
	void AIMatchAngVelObjSpace(const vector3d &desiredAngVel, const vector3d &powerLimit = vector3d(1.0), bool ignoreZeroValues = false)
	{
		m_propulsion->AIMatchAngVelObjSpace(desiredAngVel, powerLimit, ignoreZeroValues);
	}
};

#endif /* _SHIP_H */
