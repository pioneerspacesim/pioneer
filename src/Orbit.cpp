// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Orbit.h"

#include "MathUtil.h"
#include "gameconsts.h"

#ifdef _MSC_VER
#include "win32/WinMath.h"
#endif

double Orbit::OrbitalPeriod(double semiMajorAxis, double centralMass)
{
	return 2.0 * M_PI * sqrt((semiMajorAxis * semiMajorAxis * semiMajorAxis) / (G * centralMass));
}

double Orbit::OrbitalPeriodTwoBody(double semiMajorAxis, double totalMass, double bodyMass)
{
	// variable names according to the formula in:
	// http://en.wikipedia.org/wiki/Barycentric_coordinates_(astronomy)#Two-body_problem
	//
	// We have a 2-body orbital system, represented as a gravpoint (at the barycentre),
	// plus two bodies, each orbiting that gravpoint.
	// We need to compute the orbital period, given the semi-major axis of one body's orbit
	// around the gravpoint, the total mass of the system, and the mass of the body.
	//
	// According to Kepler, the orbital period P is defined by:
	//
	// P = 2*pi * sqrt( a**3 / G*(M1 + M2) )
	//
	// where a is the semi-major axis of the orbit, M1 is the mass of the primary and M2 is
	// the mass of the secondary. But we don't have that semi-major axis value, we have the
	// the semi-major axis for the orbit of the secondary around the gravpoint, instead.
	//
	// So, this first computes the semi-major axis of the secondary's orbit around the primary,
	// and then uses the above formula to compute the orbital period.
	const double r1 = semiMajorAxis;
	const double m2 = (totalMass - bodyMass);
	const double a = r1 * totalMass / m2;
	const double a3 = a * a * a;
	return 2.0 * M_PI * sqrt(a3 / (G * totalMass));
}

static double calc_velocity_area_per_sec(double semiMajorAxis, double centralMass, double eccentricity)
{
	const double a2 = semiMajorAxis * semiMajorAxis;
	const double e2 = eccentricity * eccentricity;
	return M_PI * a2 * sqrt((eccentricity < 1.0) ? (1 - e2) : (e2 - 1.0)) / Orbit::OrbitalPeriod(semiMajorAxis, centralMass);
}

static double calc_velocity_area_per_sec_gravpoint(double semiMajorAxis, double totalMass, double bodyMass, double eccentricity)
{
	const double a2 = semiMajorAxis * semiMajorAxis;
	const double e2 = eccentricity * eccentricity;
	return M_PI * a2 * sqrt((eccentricity < 1.0) ? (1 - e2) : (e2 - 1.0)) / Orbit::OrbitalPeriodTwoBody(semiMajorAxis, totalMass, bodyMass);
}

static void calc_position_from_mean_anomaly(const double M, const double e, const double a, double &cos_v, double &sin_v, double *r)
{
	// M is mean anomaly
	// e is eccentricity
	// a is semi-major axis

	cos_v = 0.0;
	sin_v = 0.0;
	if (r) {
		*r = 0.0;
	}

	if (e < 1.0) { // elliptic orbit
		// eccentric anomaly
		// NR method to solve for E: M = E-e*sin(E)  {Kepler's equation}
		double E = M;
		int iter;
		for (iter = 0; iter < 10; iter++) {
			double dE = (E - e * (sin(E)) - M) / (1.0 - e * cos(E));
			E = E - dE;
			if (fabs(dE) < 0.0001) break;
		}
		// method above sometimes can't find the solution
		// especially when e approaches 1
		if (iter == 10) { // most likely no solution found
			//failsafe to bisection method
			//max(E - M) == 1, so safe interval is M+-1.1
			double Emin = M - 1.1;
			double Emax = M + 1.1;
			double Ymin = Emin - e * sin(Emin) - M;
			double Y;
			for (int i = 0; i < 14; i++) { // 14 iterations for precision 0.00006
				E = (Emin + Emax) / 2;
				Y = E - e * sin(E) - M;
				if ((Ymin * Y) < 0) {
					Emax = E;
				} else {
					Ymin = Y;
					Emin = E;
				}
			}
		}

		// true anomaly (angle of orbit position)
		cos_v = (cos(E) - e) / (1.0 - e * cos(E));
		sin_v = (sqrt(1.0 - e * e) * sin(E)) / (1.0 - e * cos(E));

		// heliocentric distance
		if (r) {
			*r = a * (1.0 - e * cos(E));
		}

	} else { // parabolic or hyperbolic orbit
		// eccentric anomaly
		// NR method to solve for E: M = E-sinh(E)
		// sinh E and cosh E are solved directly, because of inherent numerical instability of tanh(k arctanh x)
		double sh = 2.0;
		for (int iter = 50; iter > 0; --iter) {
			double d_sh = (M + e * sh - asinh(sh)) / (e - 1 / sqrt(1 + (sh * sh)));
			sh = sh - d_sh;
			if (fabs(d_sh) < 0.0001) break;
		}

		double ch = sqrt(1 + sh * sh);

		// true anomaly (angle of orbit position)
		cos_v = (ch - e) / (1.0 - e * ch);
		sin_v = (sqrt(e * e - 1.0) * sh) / (e * ch - 1.0);

		if (r) { // heliocentric distance
			*r = a * (e * ch - 1.0);
		}
	}
}

double Orbit::TrueAnomalyFromMeanAnomaly(double MeanAnomaly) const
{
	double cos_v, sin_v;
	calc_position_from_mean_anomaly(MeanAnomaly, m_eccentricity, m_semiMajorAxis, cos_v, sin_v, 0);
	return atan2(sin_v, cos_v);
}

double Orbit::MeanAnomalyFromTrueAnomaly(double trueAnomaly) const
{
	double M_t0;
	const double e = m_eccentricity;
	if (e < 1.0) {
		M_t0 = 2.0 * atan(tan(trueAnomaly / 2.0) * sqrt((1.0 - e) / (1.0 + e)));
		M_t0 = M_t0 - e * sin(M_t0);
	} else {
		// For hyperbolic trajectories, mean anomaly has opposite sign to true anomaly, therefore trajectories which go forward
		// in time decrease their true anomaly. Yes, it is confusing.
		M_t0 = 2.0 * atanh(tan(trueAnomaly / 2.0) * sqrt((e - 1.0) / (1.0 + e)));
		M_t0 = M_t0 - e * sinh(M_t0);
	}

	return M_t0;
}

double Orbit::MeanAnomalyAtTime(double time) const
{
	const double e = m_eccentricity;
	if (e < 1.0) { // elliptic orbit
		return 2.0 * M_PI * time / Period() + m_orbitalPhaseAtStart;
	} else {
		return -2.0 * time * m_velocityAreaPerSecond / (m_semiMajorAxis * m_semiMajorAxis * sqrt(e * e - 1)) + m_orbitalPhaseAtStart;
	}
}

vector3d Orbit::OrbitalPosAtTime(double t) const
{
	if (is_zero_general(m_semiMajorAxis)) return m_positionForStaticBody;
	double cos_v, sin_v, r;
	calc_position_from_mean_anomaly(MeanAnomalyAtTime(t), m_eccentricity, m_semiMajorAxis, cos_v, sin_v, &r);
	return m_orient * vector3d(-cos_v * r, sin_v * r, 0);
}

double Orbit::OrbitalTimeAtPos(const vector3d &pos, double centralMass) const
{
	double c = m_eccentricity * m_semiMajorAxis;
	matrix3x3d matrixInv = m_orient.Inverse();
	vector3d approx3dPos = (matrixInv * pos - vector3d(c, 0., 0.)).Normalized();

	double cos_v = -vector3d(1., 0., 0.).Dot(approx3dPos);
	double sin_v = std::copysign(vector3d(1., 0., 0.).Cross(approx3dPos).Length(), approx3dPos.y);

	double cos_E = (cos_v + m_eccentricity) / (1. + m_eccentricity * cos_v);
	double E;
	double meanAnomaly;
	if (m_eccentricity <= 1.) {
		E = std::acos(cos_E);
		if (sin_v < 0)
			E *= -1.;
		meanAnomaly = E - m_eccentricity * std::sin(E);
	} else {
		E = std::acosh(cos_E);
		if (sin_v < 0)
			E *= -1.;
		meanAnomaly = E - m_eccentricity * std::sinh(E);
	}

	if (m_eccentricity <= 1.) {
		meanAnomaly -= m_orbitalPhaseAtStart;
		while (meanAnomaly < 0)
			meanAnomaly += 2. * M_PI;
	} else if (meanAnomaly < 0.)
		meanAnomaly += m_orbitalPhaseAtStart;

	if (m_eccentricity <= 1.)
		return meanAnomaly * Period() / (2. * M_PI);
	else if (meanAnomaly < 0.)
		return -meanAnomaly * std::sqrt(std::pow(m_semiMajorAxis, 3) / (G * centralMass));
	else
		return -std::fabs(meanAnomaly + m_orbitalPhaseAtStart) * std::sqrt(std::pow(m_semiMajorAxis, 3) / (G * centralMass));
}

vector3d Orbit::OrbitalVelocityAtTime(double totalMass, double t) const
{
	double cos_v, sin_v, r;
	calc_position_from_mean_anomaly(MeanAnomalyAtTime(t), m_eccentricity, m_semiMajorAxis, cos_v, sin_v, &r);

	double mi = G * totalMass;
	double p;
	if (m_eccentricity <= 1.)
		p = (1. - m_eccentricity * m_eccentricity) * m_semiMajorAxis;
	else
		p = (m_eccentricity * m_eccentricity - 1.) * m_semiMajorAxis;

	double h = std::sqrt(mi / p);

	return m_orient * vector3d(h * sin_v, h * (m_eccentricity + cos_v), 0);
}

// used for stepping through the orbit in small fractions
// mean anomaly <-> true anomaly conversion doesn't have
// to be taken into account
vector3d Orbit::EvenSpacedPosTrajectory(double t, double timeOffset) const
{
	const double e = m_eccentricity;
	double v = 2 * M_PI * t + TrueAnomalyFromMeanAnomaly(MeanAnomalyAtTime(timeOffset));
	double r;

	if (e < 1.0) {
		r = m_semiMajorAxis * (1 - e * e) / (1 + e * cos(v));
	} else {
		r = m_semiMajorAxis * (e * e - 1) / (1 + e * cos(v));

		// planet is in infinity
		const double ac = acos(-1 / e);
		if (v <= -ac) {
			v = -ac + 0.0001;
			r = 100.0 * AU;
		}
		if (v >= ac) {
			v = ac - 0.0001;
			r = 100.0 * AU;
		}
	}

	return m_orient * vector3d(-cos(v) * r, sin(v) * r, 0);
}

double Orbit::Period() const
{
	if (m_eccentricity < 1 && m_eccentricity >= 0) {
		return M_PI * m_semiMajorAxis * m_semiMajorAxis * sqrt(1 - m_eccentricity * m_eccentricity) / m_velocityAreaPerSecond;
	} else { // hyperbola.. period makes no sense, should not be used
		assert(0);
		return 0;
	}
}

vector3d Orbit::Apogeum() const
{
	if (m_eccentricity < 1) {
		return m_semiMajorAxis * (1 + m_eccentricity) * (m_orient * vector3d(1, 0, 0));
	} else {
		return vector3d(0, 0, 0);
	}
}

vector3d Orbit::Perigeum() const
{
	if (m_eccentricity < 1) {
		return m_semiMajorAxis * (1 - m_eccentricity) * (m_orient * vector3d(-1, 0, 0));
	} else {
		return m_semiMajorAxis * (m_eccentricity - 1) * (m_orient * vector3d(-1, 0, 0));
	}
}

void Orbit::SetShapeAroundBarycentre(double semiMajorAxis, double totalMass, double bodyMass, double eccentricity)
{
	m_semiMajorAxis = semiMajorAxis;
	m_eccentricity = eccentricity;
	m_velocityAreaPerSecond = calc_velocity_area_per_sec_gravpoint(semiMajorAxis, totalMass, bodyMass, eccentricity);
}

void Orbit::SetShapeAroundPrimary(double semiMajorAxis, double centralMass, double eccentricity)
{
	m_semiMajorAxis = semiMajorAxis;
	m_eccentricity = eccentricity;
	m_velocityAreaPerSecond = calc_velocity_area_per_sec(semiMajorAxis, centralMass, eccentricity);
}

Orbit Orbit::ForStaticBody(const vector3d &position)
{
	Orbit ret;
	// just remember the current position of the body, and we will return it, for any t
	ret.m_positionForStaticBody = position;
	return ret;
}

Orbit Orbit::FromBodyState(const vector3d &pos, const vector3d &vel_raw, double centralMass)
{
	Orbit ret;

	// standard gravitational parameter
	const double u = centralMass * G;

	// maybe we will adjust the speed a little now
	vector3d vel = vel_raw;
	// angular momentum
	vector3d ang = pos.Cross(vel);
	// quite a rare case - the speed is directed strictly to the star or away from the star
	// let's make a small disturbance to the velocity, so as not to calculate the radial orbit
	double speed = vel.Length();
	double dist = pos.Length();
	bool radial_orbit = is_zero_general(speed) || is_zero_general(dist) || is_zero_general(1.0 - fabs(pos.Dot(vel)) / speed / dist);
	if (radial_orbit && !is_zero_general(centralMass)) {
		if (is_zero_general(pos.x) && is_zero_general(pos.y)) // even rarer case, the body lies strictly on the z-axis
			vel.x += 1e-3 + 1e-6 * speed;
		else
			vel.z += 1e-3 + 1e-6 * speed;
		ang = pos.Cross(vel); // recalculate angular momentum
	}

	const double r_now = pos.Length();

	const double LLSqr = ang.LengthSqr();

	// total energy
	const double EE = vel.LengthSqr() / 2.0 - u / r_now;

	if (is_zero_general(centralMass) || is_zero_general(EE) || (ang.z * ang.z / LLSqr > 1.0))
		return Orbit::ForStaticBody(pos);

	// http://en.wikipedia.org/wiki/Orbital_eccentricity
	ret.m_eccentricity = 1 + 2 * EE * LLSqr / (u * u);
	if (ret.m_eccentricity < 0.0) ret.m_eccentricity = 0.0;
	ret.m_eccentricity = sqrt(ret.m_eccentricity);
	//avoid parabola
	if (ret.m_eccentricity < 1.0001 && ret.m_eccentricity >= 1) ret.m_eccentricity = 1.0001;
	if (ret.m_eccentricity > 0.9999 && ret.m_eccentricity < 1) ret.m_eccentricity = 0.9999;

	// lines represent these quantities:
	// 		(e M G)^2
	// 		M G (e - 1) / 2 EE, always positive (EE and (e-1) change sign
	// 		M G / 2 EE,
	// which is a (http://en.wikipedia.org/wiki/Semi-major_axis); a of hyperbola is taken as positive here
	ret.m_semiMajorAxis = 2 * EE * LLSqr + u * u;
	if (ret.m_semiMajorAxis < 0) ret.m_semiMajorAxis = 0;
	ret.m_semiMajorAxis = (sqrt(ret.m_semiMajorAxis) - u) / (2 * EE);
	ret.m_semiMajorAxis = ret.m_semiMajorAxis / fabs(1.0 - ret.m_eccentricity);

	// clipping of the eccentricity leads to a strong decrease in the semimajor axis.
	// at low speed, since the ship is almost in the apocenter, semimajor axis should be
	// almost equal to half distance to the star (no less that's for sure)
	if (ret.m_eccentricity < 1 && ret.m_semiMajorAxis < r_now / 2) ret.m_semiMajorAxis = r_now / 2;

	// The formulas for rotation matrix were derived based on following assumptions:
	//	1. Trajectory follows Kepler's law and vector {-r cos(v), -r sin(v), 0}, r(t) and v(t) are parameters.
	//	2. Correct transformation must transform {0,0,LL} to ang and {-r_now cos(orbitalPhaseAtStart), -r_now sin(orbitalPhaseAtStart), 0} to pos.
	//  3. orbitalPhaseAtStart (=offset) is calculated from r = a ((e^2 - 1)/(1 + e cos(v) ))
	double off = 0;

	if (ret.m_eccentricity < 1) {
		off = ret.m_semiMajorAxis * (1 - ret.m_eccentricity * ret.m_eccentricity) - r_now;
	} else {
		off = ret.m_semiMajorAxis * (-1 + ret.m_eccentricity * ret.m_eccentricity) - r_now;
	}

	// correct sign of offset is given by sign pos.Dot(vel) (heading towards apohelion or perihelion?]
	off = Clamp(off / (r_now * ret.m_eccentricity), -1.0, 1.0);
	off = -pos.Dot(vel) / fabs(pos.Dot(vel)) * acos(off);

	//much simpler and satisfies the specified conditions
	//and does not have unstable places (almost almost)
	vector3d b1 = -pos.Normalized(); //x
	vector3d b2 = -ang.Normalized(); //z
	ret.m_orient = matrix3x3d::FromVectors(b1, b2.Cross(b1), b2) * matrix3x3d::RotateZ(-off).Transpose();

	ret.m_velocityAreaPerSecond = calc_velocity_area_per_sec(ret.m_semiMajorAxis, centralMass, ret.m_eccentricity);

	ret.m_orbitalPhaseAtStart = ret.MeanAnomalyFromTrueAnomaly(-off);

	return ret;
}
