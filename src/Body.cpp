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
	return m_frame->GetPosRelativeToOtherFrame(relTo) + GetPosition();
}

const vector3d& Body::GetProjectedPos() const
{
	assert(IsOnscreen());
	return m_projectedPos;
}