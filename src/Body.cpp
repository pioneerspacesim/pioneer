#include "libs.h"
#include "Body.h"
#include "Frame.h"

Body::Body()
{
	m_frame = 0;
	m_flags = 0;
}
/* f == NULL, then absolute position within system */
vector3d Body::GetPositionRelTo(const Frame *relTo)
{
	return m_frame->GetPosRelativeToOtherFrame(relTo) + GetPosition();
}
