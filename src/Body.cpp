#include "libs.h"
#include "Body.h"
#include "Frame.h"

Body::Body()
{
	m_frame = 0;
	m_flags = 0;
	m_projectedPos = vector3d(0.0f, 0.0f, 0.0f);
	m_onscreen = false;
	m_dead = false;
}

Body::~Body()
{
	// Do not call delete body. Call Space::KillBody(body).
	assert(m_dead);
}

/* f == NULL, then absolute position within system */
vector3d Body::GetPositionRelTo(const Frame *relTo)
{
	matrix4x4d m;
	Frame::GetFrameTransform(m_frame, relTo, m);
	return m * GetPosition();
}

const vector3d& Body::GetProjectedPos() const
{
	assert(IsOnscreen());
	return m_projectedPos;
}

void Body::OrientOnSurface(double radius, double latitude, double longitude)
{
	vector3d pos = vector3d(radius*cos(latitude)*cos(longitude), radius*sin(latitude)*cos(longitude), radius*sin(longitude));
	vector3d up = vector3d::Normalize(pos);
	SetPosition(pos);

	vector3d forward = vector3d(0,0,1);
	vector3d other = vector3d::Normalize(vector3d::Cross(up, forward));
	forward = vector3d::Cross(other, up);

	matrix4x4d rot = matrix4x4d::MakeRotMatrix(other, up, forward);
	rot = rot.InverseOf();
	SetRotMatrix(rot);
}
