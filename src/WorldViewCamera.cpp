// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "WorldViewCamera.h"
#include "Ship.h"
#include "Pi.h"
#include "Game.h"
#include "AnimationCurves.h"

WorldViewCamera::WorldViewCamera(const Ship *s, const vector2f &size, float fovY, float near, float far) :
	Camera(s, size.x, size.y, fovY, near, far)
{

}

InternalCamera::InternalCamera(const Ship *s, const vector2f &size, float fovY, float near, float far) :
	WorldViewCamera(s, size, fovY, near, far)
{
	s->onFlavourChanged.connect(sigc::bind(sigc::mem_fun(this, &InternalCamera::OnShipFlavourChanged), s));
	OnShipFlavourChanged(s);
	SetBodyVisible(false);
	SetMode(MODE_FRONT);
}

void InternalCamera::OnShipFlavourChanged(const Ship *s)
{
	SetPosition(s->GetShipType().cameraOffset);
}

void InternalCamera::SetMode(Mode m)
{
	m_mode = m;
	switch (m_mode) {
		case MODE_FRONT:
			m_name = Lang::CAMERA_FRONT_VIEW;
			SetOrient(matrix3x3d::RotateY(M_PI*2));
			break;
		case MODE_REAR:
			m_name = Lang::CAMERA_REAR_VIEW;
			SetOrient(matrix3x3d::RotateY(M_PI));
			break;
		case MODE_LEFT:
			m_name = Lang::CAMERA_LEFT_VIEW;
			SetOrient(matrix3x3d::RotateY((M_PI/2)*3));
			break;
		case MODE_RIGHT:
			m_name = Lang::CAMERA_RIGHT_VIEW;
			SetOrient(matrix3x3d::RotateY(M_PI/2));
			break;
		case MODE_TOP:
			m_name = Lang::CAMERA_TOP_VIEW;
			SetOrient(matrix3x3d::RotateX((M_PI/2)*3));
			break;
		case MODE_BOTTOM:
			m_name = Lang::CAMERA_BOTTOM_VIEW;
			SetOrient(matrix3x3d::RotateX(M_PI/2));
			break;
	}
}

void InternalCamera::Save(Serializer::Writer &wr)
{
	wr.Int32(m_mode);
}

void InternalCamera::Load(Serializer::Reader &rd)
{
	SetMode(static_cast<Mode>(rd.Int32()));
}


ExternalCamera::ExternalCamera(const Ship *s, const vector2f &size, float fovY, float near, float far) :
	MoveableCamera(s, size, fovY, near, far),
	m_dist(200), m_distTo(m_dist),
	m_rotX(0),
	m_rotY(0),
	m_extOrient(matrix3x3d::Identity())
{
}

void ExternalCamera::RotateUp(float frameTime)
{
	m_rotX -= 45*frameTime;
}

void ExternalCamera::RotateDown(float frameTime)
{
	m_rotX += 45*frameTime;
}

void ExternalCamera::RotateLeft(float frameTime)
{
	m_rotY -= 45*frameTime;
}

void ExternalCamera::RotateRight(float frameTime)
{
	m_rotY += 45*frameTime;
}

void ExternalCamera::ZoomIn(float frameTime)
{
	ZoomOut(-frameTime);
}

void ExternalCamera::ZoomOut(float frameTime)
{
	m_dist += 400*frameTime;
	m_dist = std::max(GetBody()->GetClipRadius(), m_dist);
	m_distTo = m_dist;
}

void ExternalCamera::ZoomEvent(float amount)
{
	m_distTo += 400*amount;
	m_distTo = std::max(GetBody()->GetClipRadius(), m_distTo);
}

void ExternalCamera::ZoomEventUpdate(float frameTime)
{
	AnimationCurves::Approach(m_dist, m_distTo, frameTime);
	m_dist = std::max(GetBody()->GetClipRadius(), m_dist);
}

void ExternalCamera::Reset()
{
	m_dist = 200;
	m_distTo = m_dist;
}

void ExternalCamera::UpdateTransform()
{
	// when landed don't let external view look from below
	// XXX shouldn't be limited to player
	const Body *b = GetBody();
	if (b->IsType(Object::PLAYER)) {
		if (static_cast<const Ship*>(b)->GetFlightState() == Ship::LANDED ||
			static_cast<const Ship*>(b)->GetFlightState() == Ship::DOCKED) {
			m_rotX = Clamp(m_rotX, -170.0, -10.0);
		}
	}
	vector3d p = vector3d(0, 0, m_dist);
	p = matrix3x3d::RotateX(-DEG2RAD(m_rotX)) * p;
	p = matrix3x3d::RotateY(-DEG2RAD(m_rotY)) * p;
	m_extOrient = matrix3x3d::RotateY(-DEG2RAD(m_rotY)) *
		matrix3x3d::RotateX(-DEG2RAD(m_rotX));
	SetPosition(p);
	SetOrient(m_extOrient);
}

void ExternalCamera::Save(Serializer::Writer &wr)
{
	wr.Double(m_rotX);
	wr.Double(m_rotY);
	wr.Double(m_dist);
}

void ExternalCamera::Load(Serializer::Reader &rd)
{
	m_rotX = rd.Double();
	m_rotY = rd.Double();
	m_dist = rd.Double();
	m_distTo = m_dist;
}

SiderealCamera::SiderealCamera(const Ship *s, const vector2f &size, float fovY, float near, float far) :
	MoveableCamera(s, size, fovY, near, far),
	m_dist(200), m_distTo(m_dist),
	m_sidOrient(matrix3x3d::Identity())
{
}

void SiderealCamera::RotateUp(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorX();
	m_sidOrient = matrix3x3d::Rotate(-M_PI/4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCamera::RotateDown(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorX();
	m_sidOrient = matrix3x3d::Rotate(M_PI/4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCamera::RotateLeft(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorY();
	m_sidOrient = matrix3x3d::Rotate(-M_PI/4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCamera::RotateRight(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorY();
	m_sidOrient = matrix3x3d::Rotate(M_PI/4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCamera::ZoomIn(float frameTime)
{
	ZoomOut(-frameTime);
}

void SiderealCamera::ZoomOut(float frameTime)
{
	m_dist += 400*frameTime;
	m_dist = std::max(GetBody()->GetClipRadius(), m_dist);
	m_distTo = m_dist;
}

void SiderealCamera::ZoomEvent(float amount)
{
	m_distTo += 400*amount;
	m_distTo = std::max(GetBody()->GetClipRadius(), m_distTo);
}

void SiderealCamera::ZoomEventUpdate(float frameTime)
{
	AnimationCurves::Approach(m_dist, m_distTo, frameTime, 4.0, 50./std::max(m_distTo, 1e-7));		// std::max() here just avoid dividing by 0.
	m_dist = std::max(GetBody()->GetClipRadius(), m_dist);
}

void SiderealCamera::RollLeft(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorZ();
	m_sidOrient = matrix3x3d::Rotate(M_PI/4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCamera::RollRight(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorZ();
	m_sidOrient = matrix3x3d::Rotate(-M_PI/4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCamera::Reset()
{
	m_dist = 200;
	m_distTo = m_dist;
}

void SiderealCamera::UpdateTransform()
{
	m_sidOrient.Renormalize();			// lots of small rotations
	matrix3x3d shipOrient = GetBody()->GetInterpOrientRelTo(Pi::game->GetSpace()->GetRootFrame());

	SetPosition(shipOrient.Transpose() * m_sidOrient.VectorZ() * m_dist);
	SetOrient(shipOrient.Transpose() * m_sidOrient);
}

void SiderealCamera::Save(Serializer::Writer &wr)
{
	for (int i = 0; i < 9; i++) wr.Double(m_sidOrient[i]);
	wr.Double(m_dist);
}

void SiderealCamera::Load(Serializer::Reader &rd)
{
	for (int i = 0; i < 9; i++) m_sidOrient[i] = rd.Double();
	m_dist = rd.Double();
	m_distTo = m_dist;
}
