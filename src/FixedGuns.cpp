#include "FixedGuns.h"

FixedGuns::FixedGuns()
{
	//ctor
}

FixedGuns::~FixedGuns()
{
	//dtor
}

void FixedGuns::InitGun( SceneGraph::Model *m, const char *tag, int num)
{
	const SceneGraph::MatrixTransform *mt = m->FindTagByName(tag);
	if (mt) {
		const matrix4x4f &trans = mt->GetTransform();
		m_gun[num].pos = trans.GetTranslate();
		m_gun[num].dir = trans.GetOrient().VectorZ();
	} else {
		// XXX deprecated
		m_gun[num].pos = (num==FixedGuns::GUN_FRONT) ? vector3f(0,0,0) : vector3f(0,0,0);
		m_gun[num].dir = (num==FixedGuns::GUN_FRONT) ? vector3f(0,0,-1) : vector3f(0,0,1);
	}
}
