#include "WorldViewCamera.h"
#include "Ship.h"
#include "Pi.h"
#include "Game.h"

WorldViewCamera::WorldViewCamera(const Ship *s, const vector2f &size, float fovY, float near, float far) :
	Camera(s, size.x, size.y, fovY, near, far)
{

}

FrontCamera::FrontCamera(const Ship *s, const vector2f &size, float fovY, float near, float far) :
	WorldViewCamera(s, size, fovY, near, far)
{
	Activate();
}

void FrontCamera::Activate()
{
	const vector3d &offs = static_cast<const Ship*>(GetBody())->GetFrontCameraOffset();
	SetPosition(offs);
	//if offset is zero (unspecified) the camera would be in the middle of the model,
	//and it would be undesirable to render the ship
	if (offs.ExactlyEqual(vector3d(0.0)))
		m_showCameraBody = false;
}

RearCamera::RearCamera(const Ship *s, const vector2f &size, float fovY, float near, float far) :
	WorldViewCamera(s, size, fovY, near, far)
{
	SetOrientation(matrix4x4d::RotateYMatrix(M_PI));
	Activate();
}

void RearCamera::Activate()
{
	const vector3d &offs = static_cast<const Ship*>(GetBody())->GetRearCameraOffset();
	SetPosition(offs);
	if (offs.ExactlyEqual(vector3d(0.0)))
		m_showCameraBody = false;
}

ExternalCamera::ExternalCamera(const Ship *s, const vector2f &size, float fovY, float near, float far) :
	WorldViewCamera(s, size, fovY, near, far),
	m_dist(200),
	m_rotX(0),
	m_rotY(0),
	m_orient(matrix4x4d::Identity())
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
	
	m_dist -= 400*frameTime;
	m_dist = std::max(GetBody()->GetBoundingRadius(), m_dist);
}

void ExternalCamera::ZoomOut(float frameTime)
{
	m_dist += 400*frameTime;
	m_dist = std::max(GetBody()->GetBoundingRadius(), m_dist);
}

void ExternalCamera::Reset()
{
	m_dist = 200;
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
	p = matrix4x4d::RotateXMatrix(-DEG2RAD(m_rotX)) * p;
	p = matrix4x4d::RotateYMatrix(-DEG2RAD(m_rotY)) * p;
	m_orient = matrix4x4d::RotateYMatrix(-DEG2RAD(m_rotY)) *
		matrix4x4d::RotateXMatrix(-DEG2RAD(m_rotX));
	SetPosition(p);
	SetOrientation(m_orient);
}

void ExternalCamera::Save(Serializer::Writer &wr)
{
	wr.Float(float(m_rotX));
	wr.Float(float(m_rotY));
	wr.Float(float(m_dist));
}

void ExternalCamera::Load(Serializer::Reader &rd)
{
	m_rotX = rd.Float();
	m_rotY = rd.Float();
	m_dist = rd.Float();
}

SiderealCamera::SiderealCamera(const Ship *s, const vector2f &size, float fovY, float near, float far) :
	WorldViewCamera(s, size, fovY, near, far),
	m_dist(200),
	m_orient(matrix4x4d::Identity())
{
	m_prevShipOrient = s->GetTransformRelTo(Pi::game->GetSpace()->GetRootFrame());
}

void SiderealCamera::RotateUp(float frameTime)
{
	const vector3d rotAxis = m_orient * vector3d(1,0,0);
	m_orient = matrix4x4d::RotateMatrix(-M_PI/4 * frameTime, rotAxis.x, rotAxis.y, rotAxis.z)
		* m_orient;
}

void SiderealCamera::RotateDown(float frameTime)
{
	const vector3d rotAxis = m_orient * vector3d(1,0,0);
	m_orient = matrix4x4d::RotateMatrix(M_PI/4 * frameTime, rotAxis.x, rotAxis.y, rotAxis.z)
		* m_orient;
}

void SiderealCamera::RotateLeft(float frameTime)
{
	const vector3d rotAxis = m_orient * vector3d(0,1,0);
	m_orient = matrix4x4d::RotateMatrix(-M_PI/4 * frameTime, rotAxis.x, rotAxis.y, rotAxis.z)
		* m_orient;
}

void SiderealCamera::RotateRight(float frameTime)
{
	const vector3d rotAxis = m_orient * vector3d(0,1,0);
	m_orient = matrix4x4d::RotateMatrix(M_PI/4 * frameTime, rotAxis.x, rotAxis.y, rotAxis.z)
		* m_orient;
}

void SiderealCamera::ZoomIn(float frameTime)
{
	m_dist -= 400*frameTime;
	m_dist = std::max(GetBody()->GetBoundingRadius(), m_dist);
}

void SiderealCamera::ZoomOut(float frameTime)
{
	m_dist += 400*frameTime;
	m_dist = std::max(GetBody()->GetBoundingRadius(), m_dist);
}

void SiderealCamera::RollLeft(float frameTime)
{
	const vector3d rotAxis = m_orient * vector3d(0,0,1);
	m_orient = matrix4x4d::RotateMatrix(M_PI/4 * frameTime, rotAxis.x, rotAxis.y, rotAxis.z)
		* m_orient;
}

void SiderealCamera::RollRight(float frameTime)
{
	const vector3d rotAxis = m_orient * vector3d(0,0,1);
	m_orient = matrix4x4d::RotateMatrix(-M_PI/4 * frameTime, rotAxis.x, rotAxis.y, rotAxis.z)
		* m_orient;
}

void SiderealCamera::Reset()
{
	m_dist = 200;
}

void SiderealCamera::UpdateTransform()
{
	const matrix4x4d curShipOrient = static_cast<const Ship*>(GetBody())->GetInterpolatedTransformRelTo(Pi::game->GetSpace()->GetRootFrame());

	const matrix4x4d invAngDisp = curShipOrient.InverseOf() * m_prevShipOrient;
	m_orient = invAngDisp * m_orient;

	m_orient.Renormalize();
	m_orient.ClearToRotOnly();

	m_prevShipOrient = curShipOrient;

	const vector3d p = m_orient * vector3d(0, 0, m_dist);
	SetPosition(p);
	SetOrientation(m_orient);
}

void SiderealCamera::Save(Serializer::Writer &wr)
{
	for (int i = 0; i < 16; i++) wr.Float(float(m_orient[i]));
	wr.Float(float(m_dist));
}

void SiderealCamera::Load(Serializer::Reader &rd)
{
	for (int i = 0; i < 16; i++) m_orient[i] = rd.Float();
	m_dist = rd.Float();
	m_prevShipOrient = static_cast<const Ship*>(GetBody())->GetTransformRelTo(Pi::game->GetSpace()->GetRootFrame());
}
