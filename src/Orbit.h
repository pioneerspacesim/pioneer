// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef ORBIT_H
#define ORBIT_H

#include "libs.h"
#include "vector3.h"
#include "matrix3x3.h"
#include "units_h.h"

class Orbit {
public:
	// utility functions for simple calculations
	static units::time::second_t OrbitalPeriod(units::length::meter_t semiMajorAxis, units::mass::kilogram_t centralMass);
	static units::time::second_t OrbitalPeriodTwoBody(units::length::meter_t semiMajorAxis, units::mass::kilogram_t totalMass, units::mass::kilogram_t bodyMass);

	// note: the resulting Orbit is at the given position at t=0
	static Orbit FromBodyState(const vector3d &position, const vector3d &velocity, double central_mass);

	Orbit():
		m_eccentricity(0.0),
		m_semiMajorAxis(units::length::meter_t(0.0)),
		m_orbitalPhaseAtStart(units::angle::radian_t(0.0)),
		m_velocityAreaPerSecond(0.0),
		m_orient(matrix3x3d::Identity())
	{}

	void SetShapeAroundBarycentre(double semiMajorAxis, double totalMass, double bodyMass, double eccentricity);
	void SetShapeAroundPrimary(double semiMajorAxis, double totalMass, double eccentricity);
	void SetPlane(const matrix3x3d &orient) {
		m_orient = orient;
		assert(!std::isnan(m_orient[0]) && !std::isnan(m_orient[1]) && !std::isnan(m_orient[2]));
		assert(!std::isnan(m_orient[3]) && !std::isnan(m_orient[4]) && !std::isnan(m_orient[5]));
		assert(!std::isnan(m_orient[6]) && !std::isnan(m_orient[7]) && !std::isnan(m_orient[8]));
	}
	void SetPhase(units::angle::radian_t orbitalPhaseAtStart) { m_orbitalPhaseAtStart = orbitalPhaseAtStart; }

	vector3d OrbitalPosAtTime(units::time::second_t t) const;
	units::time::second_t OrbitalTimeAtPos(const vector3d& pos, units::mass::kilogram_t centralMass) const;
	vector3d OrbitalVelocityAtTime(units::mass::kilogram_t totalMass, units::time::second_t t) const;

	// 0.0 <= t <= 1.0. Not for finding orbital pos
	vector3d EvenSpacedPosTrajectory(units::time::second_t t, units::time::second_t timeOffset = units::time::second_t(0)) const;

	units::time::second_t Period() const;
	vector3d Apogeum() const;
	vector3d Perigeum() const;

	// basic accessors
	double GetEccentricity() const { return m_eccentricity; }
	units::length::meter_t GetSemiMajorAxis() const { return m_semiMajorAxis; }
	units::angle::radian_t GetOrbitalPhaseAtStart() const { return m_orbitalPhaseAtStart; }
	const matrix3x3d &GetPlane() const { return m_orient; }

private:
	units::angle::radian_t TrueAnomalyFromMeanAnomaly(units::angle::radian_t MeanAnomaly) const;
	units::angle::radian_t MeanAnomalyFromTrueAnomaly(units::angle::radian_t trueAnomaly) const;
	units::angle::radian_t MeanAnomalyAtTime(units::time::second_t time) const;

	double m_eccentricity;
	units::length::meter_t m_semiMajorAxis;
	units::angle::radian_t m_orbitalPhaseAtStart; // 0 to 2 pi radians
	/* dup " " --------------------------------------- */
	square_meters_per_second_t m_velocityAreaPerSecond; // seconds
	matrix3x3d m_orient;
};

#endif
