// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PrecalcPath.h"
#include "utils.h"

#include <algorithm>
#include <cmath>

// path element calculation functions
// all information for the calculation is taken only from the arguments
//
// notation:
//
//   units:
//     m:    mass, kg
//     F:    forward thruster power, N
//     EV:   effective exhaust velocity, m/s
//     t:    time, s
//     S:    path length, m
//     acap: acceleration limit, m/s^2
//     V:    ship velocity, m/s
//     d<X>: change in variable <X> (dm, dV, dt ...)
//
//   function names postfixes:
//     ..._norm  - normal mode, F = max, dm/dt = const, a = f(t)
//     ..._acap  - limited mode, F = f(t), dm/dt = f(t), a = const = acap
//     ..._mixed - use normal mode until F/m hits acap, then limited acceleration mode
//
//   stadia:
//     1 - acceleration
//     2 - decceleration
//     i.e. t1 - acceleration time, S1 - acceleration path, m1 - mass after acceleration e.t.c.
//     since free flight is very trivial it is not numbered
//     if no stage is specified, the calculation is the same for both stages
//
//   example:
//     S1_from_t1_norm() - calculate acceleration path from acceleration time, with no acceleration limit

using val = double;

static double t_from_dm_norm(val dm, val EV, val F) { return dm * EV / F; }

static double dm_from_t_norm(val t, val EV, val F) { return F / EV * t; }
static double dm_from_t_acap(val t, val m0, val EV, val a)
{
	return m0 - m0 / std::exp(a * t / EV);
}
static double dm_from_t_mixed(val t, val EV, val m, val F, val acap)
{
	double m_cap = F / acap;
	if (m < m_cap)
		return dm_from_t_acap(t, m, EV, acap);
	else {
		double dm_norm = dm_from_t_norm(t, EV, F);
		if (m - dm_norm > m_cap)
			return dm_norm;
		else {
			double t_norm = t_from_dm_norm(m - m_cap, EV, F);
			return m - m_cap + dm_from_t_acap(t - t_norm, m_cap, EV, acap);
		}
	}
}

static double dV_from_t_norm(val t, val EV, val m, val F)
{
	return EV * (std::log(m * EV / (m * EV - F * t)));
}
static double dV_from_t_acap(val t, val acap) { return t * acap; }
static double dV_from_t_mixed(val t, val EV, val m, val F, val acap)
{
	double m_cap = F / acap;
	if (m < m_cap)
		return dV_from_t_acap(t, acap);
	else {
		double dm_norm = dm_from_t_norm(t, EV, F);
		if (m - dm_norm > m_cap)
			return dV_from_t_norm(t, EV, m, F);
		else {
			double t_norm = t_from_dm_norm(m - m_cap, EV, F);
			double dV_norm = dV_from_t_norm(t_norm, EV, m, F);
			return dV_norm + dV_from_t_acap(t - t_norm, acap);
		}
	}
}

static double t_from_dV_norm(val dV, val EV, val m, val F)
{
	return ((-1 + std::exp(dV / EV)) * m * EV) / (std::exp(dV / EV) * F);
}
static double t_from_dV_acap(val dV, val a) { return dV / a; }
static double t_from_dV_mixed(val dV, val EV, val m, val F, val acap)
{
	double m_cap = F / acap;
	if (m < m_cap)
		return t_from_dV_acap(dV, acap);
	else {
		double t_norm = t_from_dV_norm(dV, EV, m, F);
		double dm_norm = dm_from_t_norm(t_norm, EV, F);
		if (m - dm_norm > m_cap)
			return t_norm;
		else {
			t_norm = t_from_dm_norm(m - m_cap, EV, F);
			double dV_norm = dV_from_t_norm(t_norm, EV, m, F);
			return t_norm + t_from_dV_acap(dV - dV_norm, acap);
		}
	}
}

static double S1_from_t1_norm(val t1, val V0, val EV, val m, val F)
{
	return EV * (t1 + (t1 - (m * EV) / F) * std::log((m * EV) / (-(F * t1) + m * EV))) +
		V0 * t1;
}
static double S1_from_t1_acap(val t1, val V0, val acap)
{
	return V0 * t1 + acap * t1 * t1 / 2;
}
static double S1_from_t1_mixed(val t1, val V0, val EV, val m, val F, val acap)
{
	double m_cap = F / acap;
	if (m < m_cap) {
		return S1_from_t1_acap(t1, V0, acap);
	} else {
		double dm_norm = dm_from_t_norm(t1, EV, F);
		if (m - dm_norm > m_cap)
			return S1_from_t1_norm(t1, V0, EV, m, F);
		else {
			double t_norm = t_from_dm_norm(m - m_cap, EV, F);
			double S1_norm = S1_from_t1_norm(t_norm, V0, EV, m, F);
			double Vend_norm = V0 + dV_from_t_norm(t_norm, EV, m, F);
			return S1_norm + S1_from_t1_acap(t1 - t_norm, Vend_norm, acap);
		}
	}
}

static double t_from_S_acap(val S, val V0, val a)
{
	return (-V0 + std::sqrt(2 * a * S + V0 * V0)) / a;
}

static double S2_from_t2_norm(val t2, val V0, val EV, val m, val F)
{
	return (EV * (F * t2 - m * EV) * std::log(m * EV - F * t2)) / F -
		t2 * EV * std::log(m * EV) - t2 * EV + t2 * V0 +
		(m * EV * EV * std::log(m * EV)) / F;
}
static double S2_from_t2_acap(val t2, val V0, val acap)
{
	return V0 * t2 - acap * t2 * t2 / 2;
}
static double S2_from_t2_mixed(val t2, val V0, val EV, val m, val F, val acap)
{
	double m_cap = F / acap;
	if (m < m_cap)
		return S2_from_t2_acap(t2, V0, acap);
	else {
		double dm_norm = dm_from_t_norm(t2, EV, F);
		if (m - dm_norm > m_cap)
			return S2_from_t2_norm(t2, V0, EV, m, F);
		else {
			double t2_norm = t_from_dm_norm(m - m_cap, EV, F);
			double S2_norm = S2_from_t2_norm(t2_norm, V0, EV, m, F);
			double Vend_norm = V0 - dV_from_t_norm(t2_norm, EV, m, F);
			return S2_norm + S2_from_t2_acap(t2 - t2_norm, Vend_norm, acap);
		}
	}
}

static double t2_from_S2_mixed(val S2, val V0, val EV, val m, val F, val acap,
	val eps)
{
	if (m < F / acap) {
		return t_from_S_acap(S2, V0, -acap);
	} else {
		double t2 = t_from_S_acap(S2, V0, -F / m);
		double S2_delta;
		do {
			S2_delta = S2 - S2_from_t2_mixed(t2, V0, EV, m, F, acap);
			double m_curr = m - dm_from_t_mixed(t2, EV, m, F, acap);
			t2 = t2 + t_from_S_acap(S2_delta, V0 - dV_from_t_mixed(t2, EV, m, F, acap), -std::min(acap, F / m_curr));
		} while (std::abs(S2_delta) > eps);
		return t2;
	}
}

static double Vmax_from_S_acap(val S, val Vstart, val Vend, val accel, val deccel)
{
	return std::sqrt(S + Vend * Vend / (2 * deccel) + Vstart * Vstart / (2 * accel)) / std::sqrt(1 / (2 * accel) + 1 / (2 * deccel));
}

static double S_from_Vmax_mixed(val Vmax, val Vstart, val Vend, val EV, val m, val F,
	val acap, val margin)
{
	double t1 = t_from_dV_mixed(Vmax - Vstart, EV, m, F, acap);
	double S1 = S1_from_t1_mixed(t1, Vstart, EV, m, F, acap);
	double m1 = m - dm_from_t_mixed(t1, EV, m, F, acap);
	double t2 = t_from_dV_mixed(Vmax - Vend, EV, m1, F * margin, acap * margin);
	double S2 = S2_from_t2_mixed(t2, Vmax, EV, m1, F * margin, acap * margin) + Vend * t2;
	return S1 + S2;
}

static double Vmax_from_S_mixed(val S, val Vstart, val Vend, val EV, val m, val F,
	val acap, val margin, val eps)
{
	double m_cap = F / acap;
	if (m < m_cap)
		return Vmax_from_S_acap(S, Vstart, Vend, acap, acap * margin);
	else {
		double Vmax = Vmax_from_S_acap(S, Vstart, Vend, F / m, F / m * margin);
		double Vmax_prev;
		do {
			double S_guess = S_from_Vmax_mixed(Vmax, Vstart, Vend, EV, m, F, acap, margin);
			double t1 = t_from_dV_norm(Vmax, EV, m, F);
			double m1 = m - dm_from_t_mixed(t1, EV, m, F, acap);
			Vmax_prev = Vmax;
			Vmax = Vmax_from_S_acap(S - S_guess, Vmax, Vmax, F / m1, F / m1 * margin);
		} while (std::abs(Vmax - Vmax_prev) > eps);
		return Vmax;
	}
}

static double dV_from_dm(val m0, val dm, val EV)
{
	return EV * std::log(m0 / (m0 - dm));
}

static double t1_from_S1_mixed(val S1, val V0, val EV, val m, val F, val acap,
	val eps)
{
	if (m < F / acap) {
		return t_from_S_acap(S1, V0, acap);
	} else {
		double t1 = t_from_S_acap(S1, V0, F / m);
		double S1_delta;
		do {
			S1_delta = S1_from_t1_mixed(t1, V0, EV, m, F, acap) - S1;
			double m_curr = m - dm_from_t_mixed(t1, EV, m, F, acap);
			t1 = t1 - t_from_S_acap(S1_delta, dV_from_t_mixed(t1, EV, m, F, acap) + V0, -std::min(acap, F / m_curr));
		} while (std::abs(S1_delta) > eps);
		return t1;
	}
}

//#define PRECALCPATH_TESTMODE
#ifdef PRECALCPATH_TESTMODE
enum class TestMode {
	CONSTRUCTOR,
	SET_TIME,
	SET_DIST
};

static void compare_with_simulation(
	const PrecalcPath &pp,
	const TestMode mode, // what kind of calculation we check
	double catch_param,	 // could be time or path, depending on the mode
	// route / simulation parameters
	double Stotal, // total path length, m
	double V0,	   // velocity at start
	// internal ship params:
	double EV,	  // effective exhaust velocity, m/s
	double F,	  // main (forward) thruster force , N
	double acap,  // acceleration limit (forward), m/m^2
	double mass,  // full mass at start, kg
	double fuel,  // fuel mass at start, kg
	double margin // breaking reserve
);
#endif

PrecalcPath::PrecalcPath(
	double Stotal,
	double V0,
	double EV,
	double F,
	double acap,
	double mass,
	double fuel,
	double margin) :
	Stotal(Stotal), V0(V0), EV(EV), F(F), acap(acap), mass(mass), fuel(fuel), margin(margin)
{
	// calculate full time
	double Vmax = (dV_from_dm(mass, fuel, EV) + V0) / 2;
	double Smax = S_from_Vmax_mixed(Vmax, V0, 0, EV, mass, F, acap, margin);
	if (Smax > Stotal) {
		// 2 stadia - acceleration - decceleration
		m_Vmax = Vmax_from_S_mixed(Stotal, V0, 0, EV, mass, F, acap, margin, m_eps);
		m_t1 = t_from_dV_mixed(m_Vmax - V0, EV, mass, F, acap);
		m_m1 = mass - dm_from_t_mixed(m_t1, EV, mass, F, acap);
		m_t2 = t_from_dV_mixed(m_Vmax, EV, m_m1, F * margin, acap * margin);
		m_S1 = S1_from_t1_mixed(m_t1, V0, EV, mass, F, acap);
		m_S2 = Stotal - m_S1;
	} else {
		// 3 stadia - acceleration - free flight - decceleration
		m_Vmax = Vmax;
		m_t1 = t_from_dV_mixed(m_Vmax - V0, EV, mass, F, acap);
		m_S1 = S1_from_t1_mixed(m_t1, V0, EV, mass, F, acap);
		m_m1 = mass - dm_from_t_mixed(m_t1, EV, mass, F, acap);
		m_t2 = t_from_dV_mixed(m_Vmax, EV, m_m1, F * margin, acap * margin);
		m_S2 = S2_from_t2_mixed(m_t2, m_Vmax, EV, m_m1, F * margin, acap * margin);
	}
#ifdef PRECALCPATH_TESTMODE
	compare_with_simulation(*this, TestMode::CONSTRUCTOR, 0, Stotal, V0, EV, F, acap, mass, fuel, margin);
#endif
}

void PrecalcPath::setDist(double S)
{
	m_S = S;
	if (m_S < m_S1) {
		//acceleration
		m_time = t1_from_S1_mixed(m_S, V0, EV, mass, F, acap, 1);
		m_V = dV_from_t_mixed(m_time, EV, mass, F, acap) + V0;
		m_fuel = fuel - dm_from_t_mixed(m_time, EV, mass, F, acap);
		m_state = 1;
	} else if (m_S < Stotal - m_S2) {
		//free flight (if has)
		m_V = m_Vmax;
		m_fuel = fuel - mass + m_m1;
		m_time = m_t1 + (m_S - m_S1) / m_Vmax;
		m_state = 0;
	} else {
		//deccelerating
		double S_decc = m_S - Stotal + m_S2;
		double t_decc = t2_from_S2_mixed(S_decc, m_Vmax, EV, m_m1, F * margin, acap * margin, m_eps);
		m_V = m_Vmax - dV_from_t_mixed(t_decc, EV, m_m1, F * margin, acap * margin);
		m_time = m_t1 + (Stotal - m_S1 - m_S2) / m_Vmax + t_decc;
		m_fuel = fuel - (mass - m_m1 + dm_from_t_mixed(t_decc, EV, m_m1, F * margin, acap * margin));
		m_state = -1;
	}
#ifdef PRECALCPATH_TESTMODE
	compare_with_simulation(*this, TestMode::SET_DIST, S, Stotal, V0, EV, F, acap, mass, fuel, margin);
#endif
}

void PrecalcPath::setTime(double t)
{
	m_time = t;
	if (t < m_t1) {
		// accelerating
		m_V = dV_from_t_mixed(t, EV, mass, F, acap) + V0;
		m_fuel = fuel - dm_from_t_mixed(t, EV, mass, F, acap);
		m_state = 1;
		m_S = S1_from_t1_mixed(t, V0, EV, mass, F, acap);
	} else if (t < getFullTime() - m_t2) {
		// free flight (if has)
		m_V = m_Vmax;
		m_fuel = fuel - mass + m_m1;
		m_state = 0;
		m_S = m_S1 + m_Vmax * (t - m_t1);
	} else {
		// deccelerating
		double t_decc = t - getFullTime() + m_t2;
		m_V = m_Vmax - dV_from_t_mixed(t_decc, EV, m_m1, F * margin, acap * margin);
		m_fuel = fuel - (mass - m_m1 + dm_from_t_mixed(t_decc, EV, m_m1, F * margin, acap * margin));
		m_state = -1;
		m_S = Stotal - m_S2 + S2_from_t2_mixed(t_decc, m_Vmax, EV, m_m1, F * margin, acap * margin);
	}
#ifdef PRECALCPATH_TESTMODE
	compare_with_simulation(*this, TestMode::SET_TIME, t, Stotal, V0, EV, F, acap, mass, fuel, margin);
#endif
}

#ifdef PRECALCPATH_TESTMODE
static int match_and_report(const char *s, double x1, double x2)
{
	const double tolerance = 0.1; // %
	const double base = (std::abs(x1) + std::abs(x2)) / 2;
	if (base == 0) {
		Output("%s: MATCH ERROR - both parameters a 0");
		return 1;
	} else {
		const double dev = std::abs(x1 - x2) / base * 100; // percent deviation
		Output("%s | %15.2f | %15.2f | %10.5f%% | ", s, x1, x2, dev);
		if (dev > tolerance) {
			Output("FAILED\n");
			return 1;
		} else {
			Output("OK\n");
			return 0;
		}
	}
}

static int match_and_report(const char *s, const int x1, const int x2)
{
	Output("%s | %15d | %15d |             | ", s, x1, x2);
	if (x1 != x2) {
		Output("FAILED\n");
		return 1;
	} else {
		Output("OK\n");
		return 0;
	}
}

static void compare_with_simulation(
	const PrecalcPath &pp,
	TestMode mode,		// what kind of calculation we check
	double catch_param, // could be time or path, depending on the mode
	// route / simulation parameters
	double Stotal, // total path length, m
	double V0,	   // velocity at start
	// internal ship params:
	double EV,	  // effective exhaust velocity, m/s
	double F,	  // main (forward) thruster force , N
	double acap,  // acceleration limit (forward), m/m^2
	double mass,  // full mass at start, kg
	double fuel,  // fuel mass at start, kg
	double margin // breaking reserve
)
{
	// one-dimensional space ship flight simulator
	// the entire path is simulated, with a certain time step
	// outputs are captured when t becomes greater than catch_param, or S becomes greater than catch_param

	// simulation results
	//   at constructor
	double sim_duration; // duration of the whole journey, s
	double sim_Vmax = 0; // maximum travel speed m/s
	//   at set point
	double sim_V = 0;	 // velocity at set point, m/s^2
	double sim_fuel = 0; // remaining fuel at set point, kg
	double sim_time = 0; // elapsed time at set point, s
	double sim_dist = 0; // distance from start to set point
	int sim_state = -2;	 // -1: deccelerating, 0: free flight, 1: accelerating, at set point

	// simulation parameters
	const double d_t = 0.1; // timestep, s

	// simulation variables
	double t_curr = 0;		 // current time
	double S_curr = 0;		 // distance travelled
	double V_curr = V0;		 // current velocity
	double fuel_curr = fuel; // current fuel left
	double a_curr;			 // current acceleration
	double m_curr;			 // current full mass
	int state = 1;			 // current stadia: -1: decc, 0: free, 1: acc
	bool catched = false;	 // flag is turned on when the simulator reaches the checked point
	int stadia = 0;			 // V_curr < 0; V_curr > 0 => stadia = 1; V_curr < 0 => stadia 2;
	double setmargin = 1.0;	 // current thruster power (as well as acceleration) reserve
	// extra catching parameters (for more deep debugging)
	double S2 = 0;	   // braking distance
	double m1 = 0;	   // full mass after acceleration stadia
	double t1 = 0;	   // acceleration time
	double t_norm = 0; // acceleration limiting activation time

	const double constmass = mass - fuel;
	do {
		// timestep
		m_curr = constmass + fuel_curr;
		if (state != -1) {
			double dv_left = dV_from_dm(m_curr, fuel_curr, EV);
			double t_decc = t_from_dV_mixed(V_curr, EV, m_curr, F * margin, acap * margin);
			double S_decc = S2_from_t2_mixed(t_decc, V_curr, EV, m_curr, F * margin, acap * margin);
			if (dv_left <= V_curr) {
				state = 0;
				if (m1 == 0) m1 = m_curr;
				if (t1 == 0) t1 = t_curr;
			}
			if (S_decc >= Stotal - S_curr) {
				state = -1;
				setmargin = margin;
				if (S2 == 0) S2 = Stotal - S_curr;
			}
		}
		a_curr = F / m_curr * setmargin;
		if (a_curr > acap * setmargin) {
			a_curr = acap * setmargin;
			if (t_norm == 0) t_norm = t_curr;
		}
		a_curr = a_curr * state;
		V_curr = V_curr + a_curr * d_t;
		S_curr = S_curr + V_curr * d_t + a_curr * d_t * d_t / 2;
		t_curr = t_curr + d_t;
		fuel_curr = fuel_curr - dm_from_t_norm(d_t, EV, std::abs(m_curr * a_curr));
		if (!catched &&
			((mode == TestMode::SET_TIME && catch_param <= t_curr) ||
				(mode == TestMode::SET_DIST && catch_param <= S_curr))) {
			sim_V = V_curr;
			sim_fuel = fuel_curr;
			sim_time = t_curr;
			sim_dist = S_curr;
			sim_state = state;
			catched = true;
		}
		if (V_curr > sim_Vmax) sim_Vmax = V_curr;
		if (stadia == 0 && V_curr > 0) stadia = 1;
		if (stadia == 1 && V_curr < 0) stadia = 2;
	} while (stadia < 2);
	sim_duration = t_curr;

	// check & report
	std::map<TestMode, std::string> nameof = {
		{ TestMode::CONSTRUCTOR, "PrecalcPath()" },
		{ TestMode::SET_TIME, "setTime()" },
		{ TestMode::SET_DIST, "setDist()" }
	};

	int errors = 0;
	Output("--------------------------------------------------------------------------------\n");
	Output("Testing PrecalcPath::%s\n", nameof[mode]);
	Output(".\n");
	Output("Ship parameters:\n");
	Output("EV:         %15.2f\n", EV);
	Output("F:          %15.2f\n", F);
	Output("acap:       %15.2f\n", acap);
	Output("mass:       %15.2f\n", mass);
	Output("fuel:       %15.2f\n", fuel);
	Output("margin:     %15.2f\n", margin);
	Output(".\n");
	Output("Path and point parameters:\n");
	Output("Stotal:     %15.2f\n", Stotal);
	Output("V0:         %15.2f\n", V0);
	switch (mode) {
	case TestMode::SET_TIME:
		Output("time:       %15.2f\n", catch_param);
		break;
	case TestMode::SET_DIST:
		Output("dist:       %15.2f\n", catch_param);
		break;
	default:
		break;
	}
	Output(".\n");
	Output("parameter |      calculated |       simulated |   deviation | result\n");
	Output("----------|-----------------|-----------------|-------------|-------\n");
	errors += match_and_report("Duration ", pp.getFullTime(), sim_duration);
	errors += match_and_report("Vmax     ", pp.getVmax(), sim_Vmax);
	if (mode == TestMode::SET_TIME || mode == TestMode::SET_DIST) {
		errors += match_and_report("V        ", pp.getVel(), sim_V);
		errors += match_and_report("fuel     ", pp.getMass() - constmass, sim_fuel);
		switch (mode) {
		case TestMode::SET_DIST:
			errors += match_and_report("time     ", pp.getFullTime() - pp.getEstimate(), sim_time);
			break;
		case TestMode::SET_TIME:
			errors += match_and_report("dist     ", pp.getDist(), sim_dist);
			break;
		default:
			break;
		}
		errors += match_and_report("state    ", pp.getState(), sim_state);
	}
	Output(".\n");
	if (errors > 0)
		Output("Testing FAILED with %d errors.\n", errors);
	else
		Output("Test passed successfully!\n");
	Output("--------------------------------------------------------------------------------\n");
}
#endif // PRECALCPATH_TESTMODE
