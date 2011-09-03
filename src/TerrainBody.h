#ifndef _TERRAINBODY_H
#define _TERRAINBODY_H

#include "Body.h"

class TerrainBody : public Body {
public:
	OBJDEF(TerrainBody, Body, TERRAINBODY);

protected:
	TerrainBody(SBody*);
	TerrainBody();
	virtual ~TerrainBody() {}

	void InitTerrainBody();

private:
	SBody *m_sbody;
};

#endif
