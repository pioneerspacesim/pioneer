#ifndef _FRAME_H
#define _FRAME_H

#include "libs.h"
#include <string>
#include <list>
#include "StarSystem.h"

/*
 * Frame of reference.
 */
class Frame {
public:
	Frame();
	Frame(Frame *parent, const char *label);
	Frame(Frame *parent, const char *label, unsigned int flags);
	~Frame();
	const char *GetLabel() { return m_label.c_str(); }
	void SetLabel(const char *label) { m_label = label; }
	void SetPosition(const vector3d &pos) { m_pos = pos; }
	vector3d GetPosition() { return m_pos; }
	void SetVelocity(const vector3d &vel) { m_vel = vel; }
	vector3d GetVelocity() { return m_vel; }
	void SetRadius(double radius) { m_radius = radius; }
	void RemoveChild(Frame *f);
	void AddGeom(dGeomID g) { dSpaceAdd(m_dSpaceID, g); }
	void RemoveGeom(dGeomID g) { dSpaceRemove(m_dSpaceID, g); }
	dSpaceID GetSpaceID() { return m_dSpaceID; }

	static vector3d GetFramePosRelativeToOther(const Frame *frame, const Frame *relTo);

	vector3d GetPosRelativeToOtherFrame(const Frame *relTo) const
	{
		return GetFramePosRelativeToOther(this, relTo);
	}

	bool IsLocalPosInFrame(const vector3d &pos) {
		return (pos.Length() < m_radius);
	}
	/* if parent is null then frame position is absolute */
	Frame *m_parent;
	std::list<Frame*> m_children;
	StarSystem::SBody *sBody; // points to SBodies in Pi::current_system
	
	enum { TEMP_VIEWING=1 };
private:
	void Init(Frame *parent, const char *label, unsigned int flags);
	vector3d m_pos;
	vector3d m_vel; // note we don't use this to move frame. rather,
			// orbital rails determine velocity.
	std::string m_label;
	double m_radius;
	int m_flags;
	dSpaceID m_dSpaceID;
};

#endif /* _FRAME_H */
