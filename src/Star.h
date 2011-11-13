#ifndef _STAR_H
#define _STAR_H

#include "TerrainBody.h"

class Star: public TerrainBody {
public:
	OBJDEF(Star, TerrainBody, STAR);
	Star(SBody *sbody);
	Star();
	virtual ~Star() {};

	virtual double GetClipRadius() const;
	virtual void Render(const vector3d &viewCoords, const matrix4x4d &viewTransform);
};

#endif /* _STAR_H */
