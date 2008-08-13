#include "Frame.h"
#include "Space.h"

Frame::Frame()
{
	Init(NULL, "", 0);
}

Frame::Frame(Frame *parent, const char *label)
{
	Init(parent, label, 0);
}

Frame::Frame(Frame *parent, const char *label, unsigned int flags)
{
	Init(parent, label, flags);
}

void Frame::RemoveChild(Frame *f)
{
	m_children.remove(f);
}

void Frame::Init(Frame *parent, const char *label, unsigned int flags)
{
	sBody = 0;
	m_parent = parent;
	m_flags = flags;
	m_radius = 0;
	m_pos = vector3d(0.0f);
	m_vel = vector3d(0.0);
	m_dSpaceID = dHashSpaceCreate(0);
	if (m_parent) {
		m_parent->m_children.push_back(this);
	}
	if (label) m_label = label;
}

Frame::~Frame()
{
	dSpaceDestroy(m_dSpaceID);
	for (std::list<Frame*>::iterator i = m_children.begin(); i != m_children.end(); ++i) delete *i;
}
	
vector3d Frame::GetFramePosRelativeToOther(const Frame *frame, const Frame *relTo)
{
	vector3d pos = vector3d(0,0,0);

	const Frame *f = frame;
	const Frame *root = Space::GetRootFrame();

	while ((f!=root) && (relTo != f)) {
		pos += f->m_pos;
		f = f->m_parent;
	}

	// now pos is relative to root, or to desired frame
	while (relTo != f) {
		pos -= relTo->m_pos;
		relTo = relTo->m_parent;
	}

	return pos;
}
