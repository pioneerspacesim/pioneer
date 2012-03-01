#ifndef _MODEL_H
#define _MODEL_H

#include "libs.h"
struct LmrObjParams;

class Model {
public:
	Model() {}
	virtual ~Model() { }
	virtual float GetDrawClipRadius() const = 0;
	virtual void Render(const matrix4x4f &trans, const LmrObjParams *params) = 0;
};

#endif
