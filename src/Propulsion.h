// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PROPULSION_H
#define PROPULSION_H

#include "vector3.h"
#include "libs.h"
#include "Pi.h"
#include "Game.h"
#include "Space.h"
#include "Camera.h"
#include "json/JsonUtils.h"
#include "scenegraph/Model.h"
#include "scenegraph/Thruster.h"
#include "DynamicBody.h"
#include <vector>

// Struct to be filled and passed to Propulsion
struct VectThruster_t {
	std::string model_tag, thruster_tag;
	float thrust, eev, rot_speed;
};

typedef std::map<std::string,VectThruster_t> vecThrustersMap_t;

enum Thruster { // <enum scope='ShipType' name=ShipTypeThruster prefix=THRUSTER_ public>
	THRUSTER_REVERSE,
	THRUSTER_FORWARD,
	THRUSTER_UP,
	THRUSTER_DOWN,
	THRUSTER_LEFT,
	THRUSTER_RIGHT,
	THRUSTER_MAX // <enum skip>
};

enum NacelleRest {
	NACELLE_HOR,
	NACELLE_VERT
};

class Propulsion
{
	public:
		// Inits:
		Propulsion();
		virtual ~Propulsion() {};
		void Init(DynamicBody *b, SceneGraph::Model *m, const int tank_mass, const double effExVel, const float lin_Thrust[], const float ang_Thrust );
		/* TODO:
		 * 1- Nacelle needs a way to be initialized with Init
		 * 2- Probably something (if not all) should be moved in Model.cpp
		 * 3- There're some problems keeping all together, and so something
		      should change in class hierarchy
		*/
		void AddNacelles(const vecThrustersMap_t& vThrusters);
		void SetNacelleRest(NacelleRest nr) {
			if (nr==NACELLE_VERT) m_nacRest = vector3f(0.0, 1.0, 0.0);
			if (nr==NACELLE_HOR) m_nacRest = vector3f(0.0,0.0, -1.0);
		};
		// Bonus:
		inline void SetThrustPowerMult( double p ) { m_power_mul = Clamp( p, 1.0, 3.0 );RecalculateMaxMassFlow(); }

		// Thruster functions
		double GetThrustFwd() const;
		double GetThrustRev() const;
		double GetThrustUp() const;
		double GetThrustMin() const;
		vector3d GetThrustMax(const vector3d &dir) const;

		inline double GetAccelFwd() const { return GetThrustFwd() / m_dBody->GetMass(); }
		inline double GetAccelRev() const { return GetThrustRev() / m_dBody->GetMass(); }
		inline double GetAccelUp() const { return GetThrustUp() / m_dBody->GetMass(); }
		inline double GetAccelMin() const { return GetThrustMin() / m_dBody->GetMass(); };

		inline void SetThrusterState(int axis, double level) {
			if (m_thrusterFuel <= 0.f) level = 0.0;
			m_linLevels[axis] = Clamp(level, -1.0, 1.0);
		}
		void SetThrusterState(const vector3d &levels);
		void SetAngThrusterState(const vector3d &levels);
		inline void SetAngThrusterState(int axis, double level) { m_angLevels[axis] = Clamp(level, -1.0, 1.0); }
		inline const vector3d& GetThrusterState() const { return m_linLevels; };
		inline const vector3d& GetAngThrusterState() const { return m_angLevels; }
		inline void ClearLinThrusterState() { m_linLevels = vector3d(0,0,0); }
		inline void ClearAngThrusterState() { m_angLevels = vector3d(0,0,0); }

		inline vector3d GetActualLinThrust() const { return m_linLevels * GetThrustMax( m_linLevels ) * m_power_mul; }
		inline vector3d GetActualAngThrust() const { return m_angThrust * m_angLevels * m_power_mul; }

		// Fuel
		enum FuelState { // <enum scope='Ship' name=ShipFuelStatus prefix=FUEL_ public>
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
		float GetFuelUseRate() { return m_maxMassFlow; }
		void RecalculateMaxMassFlow();
		double GetActualMassFlow();
		// available delta-V given the ship's current fuel minus reserve according to the Tsiolkovsky equation
		double GetSpeedReachedWithFuel() const;
		/* TODO: These are needed to avoid savegamebumps:
		 * are used to pass things to/from shipStats;
		 * may be better if you not expose these fields
		*/
		inline float FuelTankMassLeft() { return m_fuelTankMass * m_thrusterFuel; }
		inline void SetFuelTankMass( int fTank ) { m_fuelTankMass = fTank; }
		void Update(const float timeStep);
		inline bool IsFuelStateChanged() { return m_FuelStateChange; }

		void Render(const matrix4x4f &trans);

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

	protected:
		virtual void SaveToJson(Json::Value &jsonObj, Space *space);
		virtual void LoadFromJson(const Json::Value &jsonObj, Space *space);
	private:
		void UpdateFuel(const float timeStep);

		int m_fuelTankMass;
		float m_linThrust[THRUSTER_MAX];
		float m_angThrust;
		double m_effectiveExhaustVelocity;
		double m_thrusterFuel;	// remaining fuel 0.0-1.0
		double m_reserveFuel;	// 0-1, fuel not to touch for the current AI program
		bool m_FuelStateChange;
		vector3d m_linLevels;
		vector3d m_angLevels;

		double m_power_mul;
		DynamicBody *m_dBody;
		SceneGraph::Model *m_smodel;
		// Detached tree of normal thrusters:
		SceneGraph::Group *m_gThrusters;

		struct vectThruster_t {
			vectThruster_t():
				mtNacelle(0),
				mtThruster(0),
				vThruster(0),
				powLevel(0)
			{}
			// Pointer to the transform node of model containing the nacelles:
			SceneGraph::MatrixTransform* mtNacelle;
			// Pointer to the transform node of detached tree containing vectorial thrusters:
			SceneGraph::MatrixTransform* mtThruster;
			// Data coming from JSon file:
			const VectThruster_t* vThruster;
			// Store power level:
			float powLevel;
		};
		std::vector<vectThruster_t> m_vectThVector;
		float m_nacellesTotalThrust;
		float m_maxMassFlow;
		vector3f m_nacRest;
};

#endif // PROPULSION_H
