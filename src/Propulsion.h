#ifndef PROPULSION_H
#define PROPULSION_H

#include "vector3.h"
#include "libs.h"

class Propulsion
{
	public:
		void SetThrusterState(int axis, double level) {
			if (m_thrusterFuel <= 0.f) level = 0.0;
			m_thrusters[axis] = Clamp(level, -1.0, 1.0);
		}
		void SetThrusterState(const vector3d &levels);
		vector3d GetThrusterState() const { return m_thrusters; };
		void SetAngThrusterState(int axis, double level) { m_angThrusters[axis] = Clamp(level, -1.0, 1.0); };
		void SetAngThrusterState(const vector3d &levels);
		vector3d GetAngThrusterState() const { return m_angThrusters; };
		void ClearLinThrusterState();
		void ClearAngThrusterState();

		enum FuelState { // <enum scope='Ship' name=ShipFuelStatus prefix=FUEL_ public>
			FUEL_OK,
			FUEL_WARNING,
			FUEL_EMPTY,
		};
		FuelState GetFuelState() { return m_thrusterFuel > 0.05f ? FUEL_OK : m_thrusterFuel > 0.0f ? FUEL_WARNING : FUEL_EMPTY; }

		// fuel left, 0.0-1.0
		double GetFuel() const { return m_thrusterFuel;	}
		void SetFuel(const double f);
		double GetFuelReserve() const { return m_reserveFuel; }
		void SetFuelReserve(const double f) { m_reserveFuel = Clamp(f, 0.0, 1.0); }

		Propulsion();
		virtual ~Propulsion();
	protected:
	private:

		double m_thrusterFuel;	// remaining fuel 0.0-1.0
		double m_reserveFuel;	// 0-1, fuel not to touch for the current AI program

		vector3d m_thrusters;
		vector3d m_angThrusters;

};

#endif // PROPULSION_H
