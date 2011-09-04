#ifndef _STAR_H
#define _STAR_H

#include "TerrainBody.h"

class Star: public TerrainBody {
public:
	OBJDEF(Star, TerrainBody, STAR);
	Star(SBody *sbody);
	Star();
	virtual ~Star() {};
};

#endif /* _STAR_H */
