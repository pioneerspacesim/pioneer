// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "vector3.h"

class TransferPlanner {
public:
	enum BurnDirection {
		PROGRADE,
		NORMAL,
		RADIAL,
	};

	TransferPlanner();
	vector3d GetVel() const;
	vector3d GetOffsetVel() const;
	vector3d GetPosition() const;
	double GetStartTime() const;
	void SetPosition(const vector3d &position);
	void IncreaseFactor(), ResetFactor(), DecreaseFactor();
	void AddStartTime(double timeStep);
	void ResetStartTime();
	double GetFactor() const { return m_factor; }
	void AddDv(BurnDirection d, double dv);
	double GetDv(BurnDirection d);
	void ResetDv(BurnDirection d);
	void ResetDv();

private:
	double m_dvPrograde;
	double m_dvNormal;
	double m_dvRadial;
	double m_factor;				   // dv multiplier
	const double m_factorFactor = 5.0; // m_factor multiplier
	vector3d m_position;
	vector3d m_velocity;
	double m_startTime;
};
