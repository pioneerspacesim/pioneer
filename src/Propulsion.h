// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PROPULSION_H
#define PROPULSION_H

#include "vector3.h"
#include "libs.h"
#include "Space.h"
#include "Camera.h"
#include "json/JsonUtils.h"
#include "scenegraph/Model.h"
#include "DynamicBody.h"

enum Thruster { // <enum scope='Thruster' name=ShipTypeThruster prefix=THRUSTER_ public>
	THRUSTER_REVERSE,
	THRUSTER_FORWARD,
	THRUSTER_UP,
	THRUSTER_DOWN,
	THRUSTER_LEFT,
	THRUSTER_RIGHT,
	THRUSTER_MAX // <enum skip>
};

class Propulsion
{
	public:
		// Inits:
		Propulsion();
		virtual ~Propulsion() {};
		void Init(DynamicBody *b, SceneGraph::Model *m, const int tank_mass, const double effExVel, const float lin_Thrust[], const float ang_Thrust );

		// Bonus:
		void SetThrustPowerMult(double p, const float lin_Thrust[], const float ang_Thrust);

		// Thruster functions
		inline double GetThrustFwd() const { return -m_linThrust[THRUSTER_FORWARD]; }
		inline double GetThrustRev() const { return m_linThrust[THRUSTER_REVERSE]; }
		inline double GetThrustUp() const { return m_linThrust[THRUSTER_UP]; }
		double GetThrustMin() const;
		vector3d GetThrustMax(const vector3d &dir) const;

		inline double GetAccelFwd() const { return GetThrustFwd() / m_dBody->GetMass(); }
		inline double GetAccelRev() const { return GetThrustRev() / m_dBody->GetMass(); }
		inline double GetAccelUp() const { return GetThrustUp() / m_dBody->GetMass(); }
		inline double GetAccelMin() const { return GetThrustMin() / m_dBody->GetMass(); };
		inline double GetAccel(Thruster thruster) const { return fabs(m_linThrust[thruster] / m_dBody->GetMass()); }

		inline void SetThrusterState(int axis, double level) {
			if (m_thrusterFuel <= 0.f) level = 0.0;
			m_thrusters[axis] = Clamp(level, -1.0, 1.0);
		}
		void SetThrusterState(const vector3d &levels);
		void SetAngThrusterState(const vector3d &levels);
		inline void SetAngThrusterState(int axis, double level) { m_angThrusters[axis] = Clamp(level, -1.0, 1.0); }
		inline vector3d GetThrusterState() const { return m_thrusters; };
		inline vector3d GetAngThrusterState() const { return m_angThrusters; }
		inline void ClearLinThrusterState() { m_thrusters = vector3d(0,0,0); }
		inline void ClearAngThrusterState() { m_angThrusters = vector3d(0,0,0); }

		inline vector3d GetActualLinThrust() const { return m_thrusters * GetThrustMax( m_thrusters ); }
		inline vector3d GetActualAngThrust() const { return m_angThrust * m_angThrusters; }

		// Fuel
		enum FuelState { // <enum scope='Propulsion' name=PropulsionFuelStatus prefix=FUEL_ public>
			FUEL_OK,
			FUEL_WARNING,
			FUEL_EMPTY,
		};

		inline FuelState GetFuelState() const { return m_thrusterFuel > 0.05f ? FUEL_OK : m_thrusterFuel > 0.0f ? FUEL_WARNING : FUEL_EMPTY; }
		// fuel left, 0.0-1.0
		inline double GetFuel() const { return m_thrusterFuel;	}
		inline void SetFuel(const double f) { m_thrusterFuel = Clamp( f, 0.0, 1.0 ); }
		inline double GetFuelReserve() const { return m_reserveFuel; }
		inline void SetFuelReserve(const double f) { m_reserveFuel = Clamp( f, 0.0, 1.0 ); }
		float GetFuelUseRate();
		// available delta-V given the ship's current fuel minus reserve according to the Tsiolkovsky equation
		double GetSpeedReachedWithFuel() const;
		/* TODO: These are needed to avoid savegamebumps:
		 * are used to pass things to/from shipStats;
		 * may be better if you not expose these fields
		*/
		inline float FuelTankMassLeft() { return m_fuelTankMass * m_thrusterFuel; }
		inline void SetFuelTankMass( int fTank ) { m_fuelTankMass = fTank; }
		void UpdateFuel(const float timeStep);
		inline bool IsFuelStateChanged() { return m_FuelStateChange; }

		void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform);

		// AI on Propulsion
		void AIModelCoordsMatchAngVel(const vector3d &desiredAngVel, double softness);
		void AIModelCoordsMatchSpeedRelTo(const vector3d &v, const DynamicBody *other);
		void AIAccelToModelRelativeVelocity(const vector3d &v);

		bool AIMatchVel(const vector3d &vel);
		bool AIChangeVelBy(const vector3d &diffvel);		// acts in obj space
		vector3d AIChangeVelDir(const vector3d &diffvel);	// world space, maintain direction
		void AIMatchAngVelObjSpace(const vector3d &angvel);
		double AIFaceUpdir(const vector3d &updir, double av=0);
		double AIFaceDirection(const vector3d &dir, double av=0);
		vector3d AIGetLeadDir(const Body *target, const vector3d& targaccel, double projspeed);

  public: // was protected:
		virtual void SaveToJson(Json::Value &jsonObj, Space *space);
		virtual void LoadFromJson(const Json::Value &jsonObj, Space *space);
	private:
		int m_fuelTankMass;
		float m_linThrust[THRUSTER_MAX];
		float m_angThrust;
		double m_effectiveExhaustVelocity;
		double m_thrusterFuel;	// remaining fuel 0.0-1.0
		double m_reserveFuel;	// 0-1, fuel not to touch for the current AI program
		bool m_FuelStateChange;
		vector3d m_thrusters;
		vector3d m_angThrusters;

		const DynamicBody *m_dBody;
		SceneGraph::Model *m_smodel;
};

#endif // PROPULSION_H
