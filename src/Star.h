#ifndef _STAR_H
#define _STAR_H

#include "TerrainBody.h"

namespace Graphics { class Renderer; }

class Star: public TerrainBody {
public:
	OBJDEF(Star, TerrainBody, STAR);
	Star(SystemBody *sbody);
	Star();
	virtual ~Star() {};

	virtual double GetClipRadius() const;
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform);
};

#endif /* _STAR_H */
