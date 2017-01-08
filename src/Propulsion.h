#ifndef PROPULSION_H
#define PROPULSION_H

#include "vector3.h"
#include "libs.h"

class Propulsion
{
	public:
		// Inits:
		Propulsion();
		virtual ~Propulsion() {};
		void Init( int tank_mass, double effectiveExVel, float ang_Thrust );
		// TODO: This is here because of lack of shared enum btw ShipType and this
		void SetLinThrust( int i, float t ) { m_linThrust[i] = t; }

		// Bonus:
		inline void SetThrustPowerMult( double p ) { m_power_mul = Clamp( p, 1.0, 3.0 ); }

		// Thruster functions
		inline double GetThrustFwd() const { return -m_linThrust[THRUSTER_FORWARD]; }
		inline double GetThrustRev() const { return m_linThrust[THRUSTER_REVERSE]; }
		inline double GetThrustUp() const { return m_linThrust[THRUSTER_UP]; }
		double GetThrustMin() const;
		vector3d GetThrustMax(const vector3d &dir) const;

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
		inline vector3d GetActualAngThrust() const { return m_ang_thrust * m_angThrusters; }

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
		float GetFuelUseRate();
		double GetSpeedReachedWithFuel( double mass ) const;
		inline float FuelTankMassLeft() { return m_fuelTankMass * m_thrusterFuel; }
		void UpdateFuel(const float timeStep);
		inline bool IsFuelStateChanged() { return m_FuelStateChange; }
	protected:
	private:
		enum Thruster {
			THRUSTER_REVERSE,
			THRUSTER_FORWARD,
			THRUSTER_UP,
			THRUSTER_DOWN,
			THRUSTER_LEFT,
			THRUSTER_RIGHT,
			THRUSTER_MAX // <enum skip>
		};

		int m_fuelTankMass;
		float m_linThrust[ THRUSTER_MAX ]; // It was THRUSTER_MAX
		float m_ang_thrust;
		double m_effectiveExhaustVelocity;
		double m_thrusterFuel;	// remaining fuel 0.0-1.0
		double m_reserveFuel;	// 0-1, fuel not to touch for the current AI program
		bool m_FuelStateChange;
		vector3d m_thrusters;
		vector3d m_angThrusters;

		double m_power_mul;
};

#endif // PROPULSION_H
