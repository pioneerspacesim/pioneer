#include "WorldViewCamera.h"

WorldViewCamera::WorldViewCamera(const Body *b, const vector2f &size, float fovY, float near, float far) :
	Camera(b, size.x, size.y, fovY, near, far)
{

}

RearCamera::RearCamera(const Body *b, const vector2f &size, float fovY, float near, float far) :
	WorldViewCamera(b, size, fovY, near, far)
{
	SetOrientation(matrix4x4d::RotateYMatrix(M_PI));
}

ExternalCamera::ExternalCamera(const Body *b, const vector2f &size, float fovY, float near, float far) :
	WorldViewCamera(b, size, fovY, near, far),
	m_dist(200),
	m_orient(matrix4x4d::Identity()),
	m_rotX(0),
	m_rotY(0)
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

SiderealCamera::SiderealCamera(const Ship *b, const vector2f &size, float fovY, float near, float far) :
	WorldViewCamera(b, size, fovY, near, far),
	m_dist(200),
	m_orient(matrix4x4d::Identity())
{
	m_prevShipOrient = b->GetTransformRelTo(Pi::game->GetSpace()->GetRootFrame());
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
	//clamp dist in Update()
}

void SiderealCamera::ZoomOut(float frameTime)
{
	m_dist += 400*frameTime;
	//clamp dist in Update()
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
	m_dist = std::max(GetBody()->GetBoundingRadius(), m_dist);
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