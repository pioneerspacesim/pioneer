#ifndef _MODEL_H
#define _MODEL_H
/*
 * Abstract model base class.
 */
#include "libs.h"
#include "CollMesh.h"
#include "LmrTypes.h"
namespace Graphics { class Renderer; }

class Model {
public:
	Model() {}
	virtual ~Model() { }
	virtual float GetDrawClipRadius() const = 0;
	virtual void Render(Graphics::Renderer *r, const matrix4x4f &trans, const LmrObjParams *params) = 0;
	virtual CollMesh *CreateCollisionMesh(const LmrObjParams *p) = 0;
};

#endif
