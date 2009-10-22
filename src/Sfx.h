#ifndef _SFX_H
#define _SFX_H

#include "Body.h"

class Frame;

class Sfx {
public:
	enum TYPE { TYPE_NONE, TYPE_EXPLOSION, TYPE_DAMAGE };

	static void Add(const Body *, TYPE);
	static void TimeStepAll(const float timeStep, Frame *f);
	static void RenderAll(const Frame *f, const Frame *camFrame);
	static void Serialize(const Frame *f);
	static void Unserialize(Frame *f);

	Sfx();
	void SetPosition(vector3d p);
	vector3d GetPosition() const { return m_pos; }
private:
	static Sfx *AllocSfxInFrame(Frame *f);

	void Render(const matrix4x4d &transform);
	void TimeStepUpdate(const float timeStep);
	void Save();
	void Load();

	vector3d m_pos;
	vector3d m_vel;
	float m_age;
	enum TYPE m_type;
};

#endif /* _SFX_H */
