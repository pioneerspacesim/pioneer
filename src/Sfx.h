#ifndef _SFX_H
#define _SFX_H

#include "Body.h"

class Frame;

class Sfx: public Body {
public:
	OBJDEF(Sfx, Body, SFX);
	enum TYPE { TYPE_EXPLOSION };

	static void Add(const Body *, TYPE);

	Sfx();
	//virtual ~Sfx();
	virtual void SetPosition(vector3d p);
	virtual vector3d GetPosition() const { return m_pos; }
	virtual double GetRadius() const { return 10; }
	virtual void Render(const Frame *camFrame);
	void TimeStepUpdate(const float timeStep);

protected:
	virtual void Save();
	virtual void Load();
private:
	vector3d m_pos;
	float m_age;
	enum TYPE m_type;
};

#endif /* _SFX_H */
