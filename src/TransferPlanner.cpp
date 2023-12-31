// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TransferPlanner.h"

#include "Frame.h"
#include "Game.h"
#include "Orbit.h"
#include "Pi.h"
#include "Player.h"

#include <sstream>

TransferPlanner::TransferPlanner() :
	m_position(0., 0., 0.),
	m_velocity(0., 0., 0.)
{
	m_dvPrograde = 0.0;
	m_dvNormal = 0.0;
	m_dvRadial = 0.0;
	m_startTime = 0.0;
	m_factor = 1;
}

vector3d TransferPlanner::GetVel() const { return m_velocity + GetOffsetVel(); }

vector3d TransferPlanner::GetOffsetVel() const
{
	if (m_position.ExactlyEqual(vector3d(0., 0., 0.)))
		return vector3d(0., 0., 0.);

	const vector3d pNormal = m_position.Cross(m_velocity);

	return m_dvPrograde * m_velocity.Normalized() +
		m_dvNormal * pNormal.Normalized() +
		m_dvRadial * m_position.Normalized();
}

void TransferPlanner::AddStartTime(double timeStep)
{
	if (std::fabs(m_startTime) < 1.)
		m_startTime = Pi::game->GetTime();

	m_startTime += m_factor * timeStep;
	double deltaT = m_startTime - Pi::game->GetTime();
	if (deltaT > 0.) {
		FrameId frameId = Frame::GetFrame(Pi::player->GetFrame())->GetNonRotFrame();
		Frame *frame = Frame::GetFrame(frameId);
		Orbit playerOrbit = Orbit::FromBodyState(Pi::player->GetPositionRelTo(frameId), Pi::player->GetVelocityRelTo(frameId), frame->GetSystemBody()->GetMass());

		m_position = playerOrbit.OrbitalPosAtTime(deltaT);
		m_velocity = playerOrbit.OrbitalVelocityAtTime(frame->GetSystemBody()->GetMass(), deltaT);
	} else
		ResetStartTime();
}

void TransferPlanner::ResetStartTime()
{
	m_startTime = 0;
	Frame *frame = Frame::GetFrame(Pi::player->GetFrame());
	if (!frame || GetOffsetVel().ExactlyEqual(vector3d(0., 0., 0.))) {
		m_position = vector3d(0., 0., 0.);
		m_velocity = vector3d(0., 0., 0.);
	} else {
		frame = Frame::GetFrame(frame->GetNonRotFrame());
		m_position = Pi::player->GetPositionRelTo(frame->GetId());
		m_velocity = Pi::player->GetVelocityRelTo(frame->GetId());
	}
}

double TransferPlanner::GetStartTime() const
{
	return m_startTime < 0.0 ? 0.0 : m_startTime;
}

void TransferPlanner::AddDv(BurnDirection d, double dv)
{
	if (m_position.ExactlyEqual(vector3d(0., 0., 0.))) {
		FrameId frame = Frame::GetFrame(Pi::player->GetFrame())->GetNonRotFrame();
		m_position = Pi::player->GetPositionRelTo(frame);
		m_velocity = Pi::player->GetVelocityRelTo(frame);
		m_startTime = Pi::game->GetTime();
	}

	switch (d) {
	case PROGRADE: m_dvPrograde += m_factor * dv; break;
	case NORMAL: m_dvNormal += m_factor * dv; break;
	case RADIAL: m_dvRadial += m_factor * dv; break;
	}
}

void TransferPlanner::ResetDv(BurnDirection d)
{
	switch (d) {
	case PROGRADE: m_dvPrograde = 0; break;
	case NORMAL: m_dvNormal = 0; break;
	case RADIAL: m_dvRadial = 0; break;
	}

	if (std::fabs(m_startTime) < 1. &&
		GetOffsetVel().ExactlyEqual(vector3d(0., 0., 0.))) {
		m_position = vector3d(0., 0., 0.);
		m_velocity = vector3d(0., 0., 0.);
		m_startTime = 0.;
	}
}

void TransferPlanner::ResetDv()
{
	m_dvPrograde = 0;
	m_dvNormal = 0;
	m_dvRadial = 0;

	if (std::fabs(m_startTime) < 1.) {
		m_position = vector3d(0., 0., 0.);
		m_velocity = vector3d(0., 0., 0.);
		m_startTime = 0.;
	}
}

double TransferPlanner::GetDv(BurnDirection d)
{
	switch (d) {
	case PROGRADE: return m_dvPrograde; break;
	case NORMAL: return m_dvNormal; break;
	case RADIAL: return m_dvRadial; break;
	}
	return 0.0;
}

void TransferPlanner::IncreaseFactor(void)
{
	if (m_factor > 1000) return;
	m_factor *= m_factorFactor;
}
void TransferPlanner::ResetFactor(void) { m_factor = 1; }

void TransferPlanner::DecreaseFactor(void)
{
	if (m_factor < 0.0002) return;
	m_factor /= m_factorFactor;
}

vector3d TransferPlanner::GetPosition() const { return m_position; }

void TransferPlanner::SetPosition(const vector3d &position) { m_position = position; }
