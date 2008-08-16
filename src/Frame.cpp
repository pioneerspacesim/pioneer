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
	m_sbody = 0;
	m_parent = parent;
	m_flags = flags;
	m_radius = 0;
	m_pos = vector3d(0.0f);
	m_vel = vector3d(0.0);
	m_angVel = vector3d(0.0);
	m_orient = matrix4x4d::Identity();
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

void Frame::ApplyLeavingTransform(matrix4x4d &m) const
{
	m = matrix4x4d::Translation(m_pos) * m_orient * m;
}

void Frame::ApplyEnteringTransform(matrix4x4d &m) const
{
	m = m * m_orient.InverseOf() * matrix4x4d::Translation(-m_pos);
}

void Frame::GetFrameTransform(const Frame *fFrom, const Frame *fTo, matrix4x4d &m)
{
	matrix4x4d m2 = matrix4x4d::Identity();
	m = matrix4x4d::Identity();

	const Frame *f = fFrom;
	const Frame *root = Space::GetRootFrame();

	while ((f!=root) && (fTo != f)) {
		f->ApplyLeavingTransform(m);
		f = f->m_parent;
	}

	while (fTo != f) {
		fTo->ApplyEnteringTransform(m2);
		fTo = fTo->m_parent;
	}

	m = m2 * m;
}
	
void Frame::RotateInTimestep(double step)
{
	double ang = m_angVel.Length() * step;
	if (ang == 0) return;
	vector3d rotAxis = vector3d::Normalize(m_angVel);
	matrix4x4d rotMatrix = matrix4x4d::RotateMatrix(ang, rotAxis.x, rotAxis.y, rotAxis.z);

	m_orient = m_orient * rotMatrix;
}
