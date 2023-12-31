// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PROPULSION_H
#define PROPULSION_H

#include "DynamicBody.h"
#include "JsonFwd.h"
#include "MathUtil.h"
#include "scenegraph/Model.h"
#include "vector3.h"

class Camera;
class Space;

enum Thruster { // <enum scope='Thruster' name=ShipTypeThruster prefix=THRUSTER_ public>
	THRUSTER_REVERSE,
	THRUSTER_FORWARD,
	THRUSTER_UP,
	THRUSTER_DOWN,
	THRUSTER_LEFT,
	THRUSTER_RIGHT,
	THRUSTER_MAX // <enum skip>
};

class Propulsion : public RefCounted {
public:
	// Inits:
	Propulsion();
	virtual ~Propulsion(){};
	// Acceleration cap is infinite
	void Init(DynamicBody *b, SceneGraph::Model *m, const int tank_mass, const double effExVel, const float lin_Thrust[], const float ang_Thrust);
	void Init(DynamicBody *b, SceneGraph::Model *m, const int tank_mass, const double effExVel, const float lin_Thrust[], const float ang_Thrust, const float lin_AccelerationCap[]);

	virtual void SaveToJson(Json &jsonObj, Space *space);
	virtual void LoadFromJson(const Json &jsonObj, Space *space);

	// Bonus:
	void SetThrustPowerMult(double p, const float lin_Thrust[], const float ang_Thrust);
	void SetAccelerationCapMult(double p, const float lin_AccelerationCap[]);

	// Thrust and thruster functions
	// Everything's capped unless specified otherwise.
	double GetThrust(Thruster thruster) const; // Maximum thrust possible within acceleration cap
	vector3d GetThrust(const vector3d &dir) const;
	inline double GetThrustFwd() const { return GetThrust(THRUSTER_FORWARD); }
	inline double GetThrustRev() const { return GetThrust(THRUSTER_REVERSE); }
	inline double GetThrustUp() const { return GetThrust(THRUSTER_UP); }
	double GetThrustMin() const;

	vector3d GetThrustUncapped(const vector3d &dir) const;

	inline double GetAccel(Thruster thruster) const { return GetThrust(thruster) / m_dBody->GetMass(); }
	inline double GetAccelFwd() const { return GetAccel(THRUSTER_FORWARD); }
	inline double GetAccelRev() const { return GetAccel(THRUSTER_REVERSE); }
	inline double GetAccelUp() const { return GetAccel(THRUSTER_UP); }
	inline double GetAccelMin() const { return GetThrustMin() / m_dBody->GetMass(); }

	// Clamp thruster levels and scale them down so that a level of 1
	// corresponds to the thrust from GetThrust().
	double ClampLinThrusterState(int axis, double level) const;
	vector3d ClampLinThrusterState(const vector3d &levels) const;

	// A level of 1 corresponds to the thrust from GetThrust().
	void SetLinThrusterState(int axis, double level);
	void SetLinThrusterState(const vector3d &levels);

	inline void SetAngThrusterState(int axis, double level) { m_angThrusters[axis] = Clamp(level, -1.0, 1.0); }
	void SetAngThrusterState(const vector3d &levels);

	inline vector3d GetLinThrusterState() const { return m_linThrusters; };
	inline vector3d GetAngThrusterState() const { return m_angThrusters; }

	inline void ClearLinThrusterState() { m_linThrusters = vector3d(0, 0, 0); }
	inline void ClearAngThrusterState() { m_angThrusters = vector3d(0, 0, 0); }

	inline vector3d GetActualLinThrust() const { return m_linThrusters * GetThrustUncapped(m_linThrusters); }
	inline vector3d GetActualAngThrust() const { return m_angThrusters * m_angThrust; }

	// Fuel
	enum FuelState { // <enum scope='Propulsion' name=PropulsionFuelStatus prefix=FUEL_ public>
		FUEL_OK,
		FUEL_WARNING,
		FUEL_EMPTY,
	};

	inline FuelState GetFuelState() const
	{
		return (m_thrusterFuel > 0.05f) ?
			FUEL_OK :
			(m_thrusterFuel > 0.0f) ?
				FUEL_WARNING :
				FUEL_EMPTY;
	}
	// fuel left, 0.0-1.0
	inline double GetFuel() const { return m_thrusterFuel; }
	inline double GetFuelReserve() const { return m_reserveFuel; }
	inline void SetFuel(const double f) { m_thrusterFuel = Clamp(f, 0.0, 1.0); }
	inline void SetFuelReserve(const double f) { m_reserveFuel = Clamp(f, 0.0, 1.0); }
	float GetFuelUseRate();
	// available delta-V given the ship's current fuel minus reserve according to the Tsiolkovsky equation
	double GetSpeedReachedWithFuel() const;
	/* TODO: These are needed to avoid savegamebumps:
		 * are used to pass things to/from shipStats;
		 * may be better if you not expose these fields
		*/
	inline float FuelTankMassLeft() { return m_fuelTankMass * m_thrusterFuel; }
	inline void SetFuelTankMass(int fTank) { m_fuelTankMass = fTank; }
	void UpdateFuel(const float timeStep);
	inline bool IsFuelStateChanged() { return m_fuelStateChange; }

	void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform);

	// AI on Propulsion
	void AIModelCoordsMatchSpeedRelTo(const vector3d &v, const DynamicBody *other);
	void AIAccelToModelRelativeVelocity(const vector3d &v);
	bool AIMatchVel(const vector3d &vel, const vector3d &powerLimit = vector3d(1.0));
	bool AIChangeVelBy(const vector3d &diffvel, const vector3d &powerLimit = vector3d(1.0)); // acts in object space
	vector3d AIChangeVelDir(const vector3d &diffvel);										 // object space, maintain direction
	void AIMatchAngVelObjSpace(const vector3d &angvel, const vector3d &powerLimit = vector3d(1.0), bool ignoreZeroValues = false);
	double AIFaceUpdir(const vector3d &updir, double av = 0);
	double AIFaceDirection(const vector3d &dir, double av = 0);
	vector3d AIGetLeadDir(const Body *target, const vector3d &targaccel, double projspeed);

private:
	// Thrust and thrusters
	float m_linThrust[THRUSTER_MAX];
	float m_angThrust;
	vector3d m_linThrusters; // 0.0-1.0, thruster levels
	vector3d m_angThrusters; // 0.0-1.0
	// Used to calculate max linear thrust by limiting the thruster levels
	float m_linAccelerationCap[THRUSTER_MAX];

	// Fuel
	int m_fuelTankMass;
	double m_thrusterFuel; // 0.0-1.0, remaining fuel
	double m_reserveFuel;  // 0.0-1.0, fuel not to touch for the current AI program
	double m_effectiveExhaustVelocity;
	bool m_fuelStateChange;

	const DynamicBody *m_dBody;
	SceneGraph::Model *m_smodel;
};

#endif // PROPULSION_H
