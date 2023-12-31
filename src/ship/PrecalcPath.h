// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

class PrecalcPath {
	// initial ship and path params
	double Stotal; // total path length, m
	double V0;	   // velocity at start, m/s
	double EV;	   // effective exhaust velocity, m/s
	double F;	   // main (forward) thruster force , N
	double acap;   // acceleration limit (forward), m/m^2
	double mass;   // whole ship mass (including fuel equipment...) , kg
	double fuel;   // fuel mass, kg
	double margin; // breaking reserve, koefficient

	// current point params - defined after setTime or setDist
	double m_S;	   // distance already travelled
	double m_time; // current time
	double m_V;	   // velocity at this point, m/s^2
	double m_fuel; // remaining fuel at this point, kg
	int m_state;   // -1: deccelerating, 0: free flight, 1: accelerating

	// whole path params - defined after constructor
	double m_S1;   // acceleration length
	double m_S2;   // deccleration length
	double m_t1;   // acceleration time
	double m_t2;   // decceleration time
	double m_Vmax; // max velocity
	double m_m1;   // mass after acceleration

	// also
	double m_eps = 1; // presicion for iteration function, in result value units

public:
	PrecalcPath(
		double Stotal,
		double V0,
		double EV,
		double F,
		double acap,
		double mass,
		double fuel,
		double margin);

	// this function is available after the constructor
	double getFullTime() const { return m_t1 + m_t2 + (Stotal - m_S1 - m_S2) / m_Vmax; }

	// get current point params
	double getEstimate() const { return getFullTime() - m_time; }
	double getVel() const { return m_V; }
	double getMass() const { return mass - fuel + m_fuel; }
	double getDist() const { return m_S; }
	double getVmax() const { return m_Vmax; }
	int getState() const { return m_state; }

	// set current point of path
	// by ratio (0.0 .. 1.0) of path completion (in distance)
	void setSRatio(double ratio) { setDist(Stotal * ratio); }
	// by ratio (0.0 .. 1.0) of path completion (in time)
	void setTRatio(double ratio) { setTime(getFullTime() * ratio); }
	// by distance from start of path
	void setDist(double S);
	// by time from start
	void setTime(double t);
};
