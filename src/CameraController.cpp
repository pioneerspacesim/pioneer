// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CameraController.h"
#include "Ship.h"
#include "AnimationCurves.h"
#include "Pi.h"
#include "Game.h"

CameraController::CameraController(RefCountedPtr<CameraContext> camera, const Ship *ship) :
	m_camera(camera),
	m_ship(ship),
	m_pos(0.0),
	m_orient(matrix3x3d::Identity())
{
}

void CameraController::Reset()
{
	m_pos = vector3d(0.0);
	m_orient = matrix3x3d::Identity();
}

void CameraController::Update()
{
	m_camera->SetFrame(m_ship->GetFrame());

	// interpolate between last physics tick position and current one,
	// to remove temporal aliasing
	const matrix3x3d &m = m_ship->GetInterpOrient();
	m_camera->SetOrient(m * m_orient);
	m_camera->SetPosition(m * m_pos + m_ship->GetInterpPosition());
}


InternalCameraController::InternalCameraController(RefCountedPtr<CameraContext> camera, const Ship *ship) :
	CameraController(camera, ship),
	m_mode(MODE_FRONT)
{
	Reset();
}

static bool FillCameraPosOrient(const SceneGraph::Model *m, const char *tag, vector3d &pos, matrix3x3d &orient, matrix4x4f &trans, const matrix3x3d &fallbackOrient)
{
	matrix3x3d fixOrient(matrix3x3d::Identity());

	const SceneGraph::MatrixTransform *mt = m->FindTagByName(tag);
	if (!mt)
		fixOrient = fallbackOrient;
	else
		// camera points are have +Z pointing out of the ship, X left, so we
		// have to rotate 180 about Y to get them to -Z forward, X right like
		// the rest of the ship. this is not a bug, but rather a convenience to
		// modellers. it makes sense to orient the camera point in the
		// direction the camera will face
		trans = mt->GetTransform() * matrix4x4f::RotateYMatrix(M_PI);

	pos = vector3d(trans.GetTranslate());

	// XXX sigh, this madness has to stop
	const matrix3x3f tagOrient = trans.GetOrient();
	matrix3x3d tagOrientd;
	matrix3x3ftod(tagOrient,tagOrientd);
	orient = fixOrient * tagOrientd;

	return true;
}

void InternalCameraController::Reset()
{
	CameraController::Reset();

	const SceneGraph::Model *m = GetShip()->GetModel();

	matrix4x4f fallbackTransform = matrix4x4f::Translation(vector3f(0.0));
	const SceneGraph::MatrixTransform *fallback = m->FindTagByName("tag_camera");
	if (fallback)
		fallbackTransform = fallback->GetTransform() * matrix4x4f::RotateYMatrix(M_PI);
	else
		fallbackTransform = matrix4x4f::Translation(vector3f(GetShip()->GetShipType()->cameraOffset)); // XXX deprecated

	FillCameraPosOrient(m, "tag_camera_front",  m_frontPos,  m_frontOrient,  fallbackTransform, matrix3x3d::Identity());
	FillCameraPosOrient(m, "tag_camera_rear",   m_rearPos,   m_rearOrient,   fallbackTransform, matrix3x3d::RotateY(M_PI));
	FillCameraPosOrient(m, "tag_camera_left",   m_leftPos,   m_leftOrient,   fallbackTransform, matrix3x3d::RotateY((M_PI/2)*3));
	FillCameraPosOrient(m, "tag_camera_right",  m_rightPos,  m_rightOrient,  fallbackTransform, matrix3x3d::RotateY(M_PI/2));
	FillCameraPosOrient(m, "tag_camera_top",    m_topPos,    m_topOrient,    fallbackTransform, matrix3x3d::RotateX((M_PI/2)*3));
	FillCameraPosOrient(m, "tag_camera_bottom", m_bottomPos, m_bottomOrient, fallbackTransform, matrix3x3d::RotateX(M_PI/2));

	SetMode(m_mode);
}

void InternalCameraController::SetMode(Mode m)
{
	m_mode = m;
	switch (m_mode) {
		case MODE_FRONT:
			m_name = Lang::CAMERA_FRONT_VIEW;
			SetPosition(m_frontPos);
			SetOrient(m_frontOrient);
			break;
		case MODE_REAR:
			m_name = Lang::CAMERA_REAR_VIEW;
			SetPosition(m_rearPos);
			SetOrient(m_rearOrient);
			break;
		case MODE_LEFT:
			m_name = Lang::CAMERA_LEFT_VIEW;
			SetPosition(m_leftPos);
			SetOrient(m_leftOrient);
			break;
		case MODE_RIGHT:
			m_name = Lang::CAMERA_RIGHT_VIEW;
			SetPosition(m_rightPos);
			SetOrient(m_rightOrient);
			break;
		case MODE_TOP:
			m_name = Lang::CAMERA_TOP_VIEW;
			SetPosition(m_topPos);
			SetOrient(m_topOrient);
			break;
		case MODE_BOTTOM:
			m_name = Lang::CAMERA_BOTTOM_VIEW;
			SetPosition(m_bottomPos);
			SetOrient(m_bottomOrient);
			break;
	}
}

void InternalCameraController::Save(Serializer::Writer &wr)
{
	wr.Int32(m_mode);
}

void InternalCameraController::Load(Serializer::Reader &rd)
{
	SetMode(static_cast<Mode>(rd.Int32()));
}


ExternalCameraController::ExternalCameraController(RefCountedPtr<CameraContext> camera, const Ship *ship) :
	MoveableCameraController(camera, ship),
	m_dist(200), m_distTo(m_dist),
	m_rotX(0),
	m_rotY(0),
	m_extOrient(matrix3x3d::Identity())
{
}

void ExternalCameraController::RotateUp(float frameTime)
{
	m_rotX -= 45*frameTime;
}

void ExternalCameraController::RotateDown(float frameTime)
{
	m_rotX += 45*frameTime;
}

void ExternalCameraController::RotateLeft(float frameTime)
{
	m_rotY -= 45*frameTime;
}

void ExternalCameraController::RotateRight(float frameTime)
{
	m_rotY += 45*frameTime;
}

void ExternalCameraController::ZoomIn(float frameTime)
{
	ZoomOut(-frameTime);
}

void ExternalCameraController::ZoomOut(float frameTime)
{
	m_dist += 400*frameTime;
	m_dist = std::max(GetShip()->GetClipRadius(), m_dist);
	m_distTo = m_dist;
}

void ExternalCameraController::ZoomEvent(float amount)
{
	m_distTo += 400*amount;
	m_distTo = std::max(GetShip()->GetClipRadius(), m_distTo);
}

void ExternalCameraController::ZoomEventUpdate(float frameTime)
{
	AnimationCurves::Approach(m_dist, m_distTo, frameTime);
	m_dist = std::max(GetShip()->GetClipRadius(), m_dist);
}

void ExternalCameraController::Reset()
{
	m_dist = 200;
	m_distTo = m_dist;
}

void ExternalCameraController::Update()
{
	// when landed don't let external view look from below
	// XXX shouldn't be limited to player
	const Ship *ship = GetShip();
	if (ship->IsType(Object::PLAYER)) {
		if (ship->GetFlightState() == Ship::LANDED ||
			ship->GetFlightState() == Ship::DOCKED) {
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

	CameraController::Update();
}

void ExternalCameraController::Save(Serializer::Writer &wr)
{
	wr.Double(m_rotX);
	wr.Double(m_rotY);
	wr.Double(m_dist);
}

void ExternalCameraController::Load(Serializer::Reader &rd)
{
	m_rotX = rd.Double();
	m_rotY = rd.Double();
	m_dist = rd.Double();
	m_distTo = m_dist;
}


SiderealCameraController::SiderealCameraController(RefCountedPtr<CameraContext> camera, const Ship *ship) :
	MoveableCameraController(camera, ship),
	m_dist(200), m_distTo(m_dist),
	m_sidOrient(matrix3x3d::Identity())
{
}

void SiderealCameraController::RotateUp(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorX();
	m_sidOrient = matrix3x3d::Rotate(-M_PI/4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCameraController::RotateDown(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorX();
	m_sidOrient = matrix3x3d::Rotate(M_PI/4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCameraController::RotateLeft(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorY();
	m_sidOrient = matrix3x3d::Rotate(-M_PI/4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCameraController::RotateRight(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorY();
	m_sidOrient = matrix3x3d::Rotate(M_PI/4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCameraController::ZoomIn(float frameTime)
{
	ZoomOut(-frameTime);
}

void SiderealCameraController::ZoomOut(float frameTime)
{
	m_dist += 400*frameTime;
	m_dist = std::max(GetShip()->GetClipRadius(), m_dist);
	m_distTo = m_dist;
}

void SiderealCameraController::ZoomEvent(float amount)
{
	m_distTo += 400*amount;
	m_distTo = std::max(GetShip()->GetClipRadius(), m_distTo);
}

void SiderealCameraController::ZoomEventUpdate(float frameTime)
{
	AnimationCurves::Approach(m_dist, m_distTo, frameTime, 4.0, 50./std::max(m_distTo, 1e-7));		// std::max() here just avoid dividing by 0.
	m_dist = std::max(GetShip()->GetClipRadius(), m_dist);
}

void SiderealCameraController::RollLeft(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorZ();
	m_sidOrient = matrix3x3d::Rotate(M_PI/4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCameraController::RollRight(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorZ();
	m_sidOrient = matrix3x3d::Rotate(-M_PI/4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCameraController::Reset()
{
	m_dist = 200;
	m_distTo = m_dist;
}

void SiderealCameraController::Update()
{
	const Ship *ship = GetShip();

	m_sidOrient.Renormalize();			// lots of small rotations
	matrix3x3d shipOrient = ship->GetInterpOrientRelTo(Pi::game->GetSpace()->GetRootFrame());

	SetPosition(shipOrient.Transpose() * m_sidOrient.VectorZ() * m_dist);
	SetOrient(shipOrient.Transpose() * m_sidOrient);

	CameraController::Update();
}

void SiderealCameraController::Save(Serializer::Writer &wr)
{
	for (int i = 0; i < 9; i++) wr.Double(m_sidOrient[i]);
	wr.Double(m_dist);
}

void SiderealCameraController::Load(Serializer::Reader &rd)
{
	for (int i = 0; i < 9; i++) m_sidOrient[i] = rd.Double();
	m_dist = rd.Double();
	m_distTo = m_dist;
}
