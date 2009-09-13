#ifndef _RENDER_H
#define _RENDER_H

#include "libs.h"

/*
 * bunch of reused 3d drawy routines.
 */
namespace Render {
	void PutPointSprites(int num, vector3f v[], float size, const float modulationCol[4], GLuint tex);
};

#endif /* _RENDER_H */
