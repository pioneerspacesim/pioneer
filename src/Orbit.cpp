// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Orbit.h"
#include "libs.h"
#include "gameconsts.h"

#include "units_cpp.h"
#ifdef _MSC_VER
#include "win32/WinMath.h"
#endif

unit_t<compound_unit<cubed<length::meters>, inverse<mass::kilogram>, inverse<squared<time::seconds>>>> gravconst(G);

second_t Orbit::OrbitalPeriod(meter_t semiMajorAxis, kilogram_t centralMass)
{
	return 2.0 * M_PI
		* units::math::sqrt((semiMajorAxis * semiMajorAxis * semiMajorAxis)
							/ (gravconst * centralMass));
}

second_t
Orbit::OrbitalPeriodTwoBody(meter_t semiMajorAxis, kilogram_t totalMass, kilogram_t bodyMass)
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
	const meter_t r1 = semiMajorAxis;
	const kilogram_t m2 = (totalMass - bodyMass);
	const meter_t a = r1 * totalMass / m2;
	const cubic_meter_t a3 = a * a * a;
	return 2.0 * M_PI * units::math::sqrt(a3 / (gravconst * totalMass));
}

static square_meters_per_second_t
calc_velocity_area_per_sec(meter_t semiMajorAxis, kilogram_t centralMass, double eccentricity)
{
	const square_meter_t a2 = semiMajorAxis * semiMajorAxis;
	const double e2 = eccentricity * eccentricity;
	return M_PI * a2 * sqrt((eccentricity < 1.0) ? (1 - e2) : (e2 - 1.0))
		/ Orbit::OrbitalPeriod(semiMajorAxis, centralMass);
}

static square_meters_per_second_t calc_velocity_area_per_sec_gravpoint(meter_t semiMajorAxis,
                                                                       kilogram_t totalMass,
                                                                       kilogram_t bodyMass,
                                                                       double eccentricity)
{
	const square_meter_t a2 = semiMajorAxis * semiMajorAxis;
	const double e2 = eccentricity * eccentricity;
	return M_PI * a2 * sqrt((eccentricity < 1.0) ? (1 - e2) : (e2 - 1.0))
		/ Orbit::OrbitalPeriodTwoBody(semiMajorAxis, totalMass, bodyMass);
}

static void calc_position_from_mean_anomaly(
											const double M, const double e, const double a, double &cos_v, double &sin_v, double *r)
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
		for (int iter = 5; iter > 0; --iter) {
			E = E - (E - e * (std::sin(E)) - M) / (1.0 - e * std::cos(E));
		}

		// true anomaly (angle of orbit position)
		cos_v = (std::cos(E) - e) / (1.0 - e * std::cos(E));
		sin_v = (sqrt(1.0 - e * e) * std::sin(E)) / (1.0 - e * std::cos(E));

		// heliocentric distance
		if (r) {
			*r = a * (1.0 - e * std::cos(E));
		}

	} else { // parabolic or hyperbolic orbit
		// eccentric anomaly
		// NR method to solve for E: M = E-sinh(E)
		// sinh E and cosh E are solved directly, because of inherent numerical instability of
		// tanh(k arctanh x)
		double sh = 2.0;
		for (int iter = 5; iter > 0; --iter) {
			sh = sh - (M + e * sh - asinh(sh)) / (e - 1 / sqrt(1 + (sh * sh)));
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

radian_t Orbit::TrueAnomalyFromMeanAnomaly(radian_t MeanAnomaly) const
{
	double cos_v, sin_v;
	calc_position_from_mean_anomaly(MeanAnomaly.to<double>(), m_eccentricity, m_semiMajorAxis.to<double>(),
	                                cos_v, sin_v, 0);
	return radian_t(atan2(sin_v, cos_v));
}

radian_t Orbit::MeanAnomalyFromTrueAnomaly(radian_t trueAnomaly) const
{
	radian_t M_t0;
	const double e = m_eccentricity;
	if (e < 1.0) {
		M_t0 = 2.0 * units::math::atan(units::math::tan(trueAnomaly / 2.0) * sqrt((1.0 - e) / (1.0 + e)));
		// TODO: why is radian_t needed here and below?
		M_t0 = M_t0 - radian_t(e * units::math::sin(M_t0).to<double>());
	} else {
		// For hyperbolic trajectories, mean anomaly has opposite sign to true anomaly, therefore
		// trajectories which go forward in time decrease their true anomaly. Yes, it is confusing.
		M_t0 = 2.0 * units::math::atanh(units::math::tan(trueAnomaly / 2.0) * sqrt((e - 1.0) / (1.0 + e)));
		M_t0 = M_t0 - radian_t(e * units::math::sinh(M_t0).to<double>());
	}

	return M_t0;
}

radian_t Orbit::MeanAnomalyAtTime(second_t time) const
{
	const double e = m_eccentricity;
	if (e < 1.0) { // elliptic orbit
		return 2.0 * radian_t(M_PI) * time / Period() + m_orbitalPhaseAtStart;
	} else {
		// TODO: why is radian_t necessary here?
		return radian_t(-2.0) * time * m_velocityAreaPerSecond
			/ (m_semiMajorAxis * m_semiMajorAxis * sqrt(e * e - 1))
			+ m_orbitalPhaseAtStart;
	}
}

vector3d Orbit::OrbitalPosAtTime(second_t t) const
{
	double cos_v, sin_v, r;
	calc_position_from_mean_anomaly(MeanAnomalyAtTime(t).to<double>(), m_eccentricity,
	                                m_semiMajorAxis.to<double>(), cos_v, sin_v, &r);
	return m_orient * vector3d(-cos_v * r, sin_v * r, 0);
}

second_t Orbit::OrbitalTimeAtPos(const vector3d &pos, kilogram_t centralMass) const
{
	double c = m_eccentricity * m_semiMajorAxis.to<double>();
	matrix3x3d matrixInv = m_orient.Inverse();
	vector3d approx3dPos = (matrixInv * pos - vector3d(c, 0., 0.)).Normalized();

	double cos_v = -vector3d(1., 0., 0.).Dot(approx3dPos);
	double sin_v = std::copysign(vector3d(1., 0., 0.).Cross(approx3dPos).Length(), approx3dPos.y);

	double cos_E = (cos_v + m_eccentricity) / (1. + m_eccentricity * cos_v);
	double E;
	radian_t meanAnomaly;
	if (m_eccentricity <= 1.) {
		E = std::acos(cos_E);
		if (sin_v < 0)
			E *= -1.;
		meanAnomaly = radian_t(E) - m_eccentricity * radian_t(std::sin(E));
	} else {
		E = std::acosh(cos_E);
		if (sin_v < 0)
			E *= -1.;
		meanAnomaly = radian_t(E) - m_eccentricity * radian_t(std::sinh(E));
	}

	if (m_eccentricity <= 1.) {
		meanAnomaly -= m_orbitalPhaseAtStart;
		while (meanAnomaly < radian_t(0))
			meanAnomaly += 2. * radian_t(M_PI);
	} else if (meanAnomaly < radian_t(0))
		meanAnomaly += m_orbitalPhaseAtStart;

	if (m_eccentricity <= 1.)
		return meanAnomaly * Period() / (2. * radian_t(M_PI));
	else if (meanAnomaly < 0_rad)
		// TODO: .to<double>() should be eliminated here and below
		return -meanAnomaly.to<double>()
			* units::math::sqrt(units::math::pow<3>(m_semiMajorAxis)
								/ (gravconst * centralMass));
	else
		return -units::math::fabs(meanAnomaly + m_orbitalPhaseAtStart).to<double>()
			* units::math::sqrt(units::math::pow<3>(m_semiMajorAxis) / (gravconst * centralMass));
}

vector3d Orbit::OrbitalVelocityAtTime(kilogram_t totalMass, second_t t) const
{
	double cos_v, sin_v, r;
	calc_position_from_mean_anomaly(MeanAnomalyAtTime(t).to<double>(), m_eccentricity,
	                                m_semiMajorAxis.to<double>(), cos_v, sin_v, &r);

	auto mi = gravconst * totalMass;
	meter_t p;
	if (m_eccentricity <= 1.)
		p = (1. - m_eccentricity * m_eccentricity) * m_semiMajorAxis;
	else
		p = (m_eccentricity * m_eccentricity - 1.) * m_semiMajorAxis;

	meters_per_second_t h = units::math::sqrt(mi / p);

	return m_orient * vector3d(h.to<double>() * sin_v, h.to<double>() * (m_eccentricity + cos_v), 0);
}

// used for stepping through the orbit in small fractions
// mean anomaly <-> true anomaly conversion doesn't have
// to be taken into account
vector3d Orbit::EvenSpacedPosTrajectory(second_t t, second_t timeOffset) const
{
	const double e = m_eccentricity;
	// TODO: .to<double>() should not be used here
	radian_t v = 2 * radian_t(M_PI) * t.to<double>() + TrueAnomalyFromMeanAnomaly(MeanAnomalyAtTime(timeOffset));
	meter_t r;

	if (e < 1.0) {
		r = m_semiMajorAxis * (1 - e * e) / (1 + e * units::math::cos(v));
	} else {
		r = m_semiMajorAxis * (e * e - 1) / (1 + e * units::math::cos(v));

		// planet is in infinity
		const radian_t ac = radian_t(acos(-1 / e));
		if (v <= -ac) {
			v = -ac + radian_t(0.0001);
			r = 100.0_au;
		}
		if (v >= ac) {
			v = ac - radian_t(0.0001);
			r = 100.0_au;
		}
	}

	return m_orient * vector3d(-units::math::cos(v) * r.to<double>(), units::math::sin(v) * r.to<double>(), 0);
}

second_t Orbit::Period() const
{
	if (m_eccentricity < 1 && m_eccentricity >= 0) {
		return M_PI * m_semiMajorAxis * m_semiMajorAxis * sqrt(1 - m_eccentricity * m_eccentricity)
			/ m_velocityAreaPerSecond;
	} else { // hyperbola.. period makes no sense, should not be used
		assert(0);
		return second_t(0);
	}
}

vector3d Orbit::Apogeum() const
{
	if (m_eccentricity < 1) {
		return m_semiMajorAxis.to<double>() * (1 + m_eccentricity) * (m_orient * vector3d(1, 0, 0));
	} else {
		return vector3d(0, 0, 0);
	}
}

vector3d Orbit::Perigeum() const
{
	if (m_eccentricity < 1) {
		return m_semiMajorAxis.to<double>() * (1 - m_eccentricity)
			* (m_orient * vector3d(-1, 0, 0));
	} else {
		return m_semiMajorAxis.to<double>() * (m_eccentricity - 1)
			* (m_orient * vector3d(-1, 0, 0));
	}
}

void Orbit::SetShapeAroundBarycentre(double semiMajorAxis,
                                     double totalMass,
                                     double bodyMass,
                                     double eccentricity)
{
	m_semiMajorAxis = meter_t(semiMajorAxis);
	m_eccentricity = eccentricity;
	m_velocityAreaPerSecond = calc_velocity_area_per_sec_gravpoint(
																   meter_t(semiMajorAxis), kilogram_t(totalMass), kilogram_t(bodyMass), eccentricity);
}

void Orbit::SetShapeAroundPrimary(double semiMajorAxis, double centralMass, double eccentricity)
{
	m_semiMajorAxis = meter_t(semiMajorAxis);
	m_eccentricity = eccentricity;
	m_velocityAreaPerSecond
	    = calc_velocity_area_per_sec(meter_t(semiMajorAxis), kilogram_t(centralMass), eccentricity);
}

Orbit Orbit::FromBodyState(const vector3d &pos, const vector3d &vel, double centralMass)
{
	Orbit ret;

	const double r_now = pos.Length() + 1e-12;
	const double v_now = vel.Length() + 1e-12;

	// standard gravitational parameter
	const double u = centralMass * G;

	// angular momentum
	const vector3d ang = pos.Cross(vel);
	const double LLSqr = ang.LengthSqr();
	const double LL = ang.Length();

	// total energy
	const double EE = vel.LengthSqr() / 2.0 - u / r_now;

	if (is_zero_general(centralMass) || is_zero_general(r_now) || is_zero_general(v_now)
	    || is_zero_general(EE) || (ang.z * ang.z / LLSqr > 1.0)) {
		ret.m_eccentricity = 0.0;
		ret.m_semiMajorAxis = 0.0_m;
		ret.m_velocityAreaPerSecond = square_meters_per_second_t(0.0);
		ret.m_orbitalPhaseAtStart = radian_t(0.0);
		ret.m_orient = matrix3x3d::Identity();
		return ret;
	}

	// http://en.wikipedia.org/wiki/Orbital_eccentricity
	ret.m_eccentricity = 1 + 2 * EE * LLSqr / (u * u);
	if (ret.m_eccentricity < 0.0)
		ret.m_eccentricity = 0.0;
	ret.m_eccentricity = sqrt(ret.m_eccentricity);

	// lines represent these quantities:
	// 		(e M G)^2
	// 		M G (e - 1) / 2 EE, always positive (EE and (e-1) change sign
	// 		M G / 2 EE,
	// which is a (http://en.wikipedia.org/wiki/Semi-major_axis); a of hyperbola is taken as
	// positive here
	ret.m_semiMajorAxis = meter_t(2 * EE * LLSqr + u * u);
	if (ret.m_semiMajorAxis < 0_m)
		ret.m_semiMajorAxis = 0_m;
	ret.m_semiMajorAxis
	    = meter_t((units::math::sqrt(ret.m_semiMajorAxis).to<double>() - u) / (2 * EE));
	ret.m_semiMajorAxis
	    = meter_t(ret.m_semiMajorAxis.to<double>() / fabs(1.0 - ret.m_eccentricity));

	// The formulas for rotation matrix were derived based on following assumptions:
	//	1. Trajectory follows Kepler's law and vector {-r cos(v), -r sin(v), 0}, r(t) and v(t) are
	//parameters.
	//	2. Correct transformation must transform {0,0,LL} to ang and {-r_now
	//cos(orbitalPhaseAtStart), -r_now sin(orbitalPhaseAtStart), 0} to pos.
	//  3. orbitalPhaseAtStart (=offset) is calculated from r = a ((e^2 - 1)/(1 + e cos(v) ))
	const double angle1 = acos(Clamp(ang.z / LL, -1 + 1e-6, 1 - 1e-6)) * (ang.x > 0 ? -1 : 1);
	const double angle2
	    = asin(Clamp(ang.y / (LL * sqrt(1.0 - ang.z * ang.z / LLSqr)), -1 + 1e-6, 1 - 1e-6))
		* (ang.x > 0 ? -1 : 1);

	// There are two possible solutions of the equation and the only way how to find the correct one
	// I know about is to try both and check if the position is transformed correctly. We minimize
	// the difference of the transformed  position and expected result.
	double value = 1e99, offset = 0, cc = 0;
	for (int i = -1; i <= 1; i += 2) {
		double off = 0, ccc = 0;
		matrix3x3d mat;

		if (ret.m_eccentricity < 1) {
			off = ret.m_semiMajorAxis.to<double>() * (1 - ret.m_eccentricity * ret.m_eccentricity)
				- r_now;
		} else {
			off = ret.m_semiMajorAxis.to<double>() * (-1 + ret.m_eccentricity * ret.m_eccentricity)
				- r_now;
		}

		// correct sign of offset is given by sign pos.Dot(vel) (heading towards apohelion or
		// perihelion?]
		off = Clamp(off / (r_now * ret.m_eccentricity), -1 + 1e-6, 1 - 1e-6);
		off = -pos.Dot(vel) / fabs(pos.Dot(vel)) * acos(off);

		ccc = acos(-pos.z / r_now / sin(angle1)) * i;
		mat = matrix3x3d::RotateZ(angle2) * matrix3x3d::RotateY(angle1)
			* matrix3x3d::RotateZ(ccc - off);

		if (((mat * vector3d(-r_now * cos(off), r_now * sin(off), 0)) - pos).Length() < value) {
			value = ((mat * vector3d(-r_now * cos(off), r_now * sin(off), 0)) - pos).Length();
			cc = ccc;
			offset = off;
		}
	}

	// matrix3x3d::RotateX(M_PI) and minus sign before offset changes solution above, derived for
	// orbits {-r cos(v), -r sin(v), 0} to {-r cos(v), -r sin(v), 0}
	ret.m_orient = matrix3x3d::RotateZ(angle2) * matrix3x3d::RotateY(angle1)
		* matrix3x3d::RotateZ(cc - offset) * matrix3x3d::RotateX(M_PI);
	ret.m_velocityAreaPerSecond = calc_velocity_area_per_sec(
															 ret.m_semiMajorAxis, kilogram_t(centralMass), ret.m_eccentricity);

	ret.m_orbitalPhaseAtStart = ret.MeanAnomalyFromTrueAnomaly(radian_t(-offset));

	return ret;
}
