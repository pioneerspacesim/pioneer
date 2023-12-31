// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef ORBIT_H
#define ORBIT_H

#include "matrix3x3.h"
#include "vector3.h"

#include <cassert>
#include <cmath>

class Orbit {
public:
	// utility functions for simple calculations
	static double OrbitalPeriod(double semiMajorAxis, double centralMass);
	static double OrbitalPeriodTwoBody(double semiMajorAxis, double totalMass, double bodyMass);

	// note: the resulting Orbit is at the given position at t=0
	static Orbit FromBodyState(const vector3d &position, const vector3d &velocity, double central_mass);
	static Orbit ForStaticBody(const vector3d &position);

	Orbit() :
		m_eccentricity(0.0),
		m_semiMajorAxis(0.0),
		m_orbitalPhaseAtStart(0.0),
		m_velocityAreaPerSecond(0.0),
		m_orient(matrix3x3d::Identity())
	{}

	void SetShapeAroundBarycentre(double semiMajorAxis, double totalMass, double bodyMass, double eccentricity);
	void SetShapeAroundPrimary(double semiMajorAxis, double totalMass, double eccentricity);
	void SetPlane(const matrix3x3d &orient)
	{
		m_orient = orient;
		assert(!std::isnan(m_orient[0]) && !std::isnan(m_orient[1]) && !std::isnan(m_orient[2]));
		assert(!std::isnan(m_orient[3]) && !std::isnan(m_orient[4]) && !std::isnan(m_orient[5]));
		assert(!std::isnan(m_orient[6]) && !std::isnan(m_orient[7]) && !std::isnan(m_orient[8]));
	}
	void SetPhase(double orbitalPhaseAtStart) { m_orbitalPhaseAtStart = orbitalPhaseAtStart; }

	vector3d OrbitalPosAtTime(double t) const;
	double OrbitalTimeAtPos(const vector3d &pos, double centralMass) const;
	vector3d OrbitalVelocityAtTime(double totalMass, double t) const;

	// 0.0 <= t <= 1.0. Not for finding orbital pos
	vector3d EvenSpacedPosTrajectory(double t, double timeOffset = 0) const;

	double Period() const;
	vector3d Apogeum() const;
	vector3d Perigeum() const;

	// basic accessors
	double GetEccentricity() const { return m_eccentricity; }
	double GetSemiMajorAxis() const { return m_semiMajorAxis; }
	double GetOrbitalPhaseAtStart() const { return m_orbitalPhaseAtStart; }
	const matrix3x3d &GetPlane() const { return m_orient; }

private:
	double TrueAnomalyFromMeanAnomaly(double MeanAnomaly) const;
	double MeanAnomalyFromTrueAnomaly(double trueAnomaly) const;
	double MeanAnomalyAtTime(double time) const;

	vector3d m_positionForStaticBody;
	double m_eccentricity;
	double m_semiMajorAxis;
	double m_orbitalPhaseAtStart; // 0 to 2 pi radians
	/* dup " " --------------------------------------- */
	double m_velocityAreaPerSecond; // seconds
	matrix3x3d m_orient;
};

#endif
