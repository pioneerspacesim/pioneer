// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MODEL_H
#define _MODEL_H
/*
 * Abstract model base class.
 */
#include "libs.h"
#include "CollMesh.h"
#include "LmrTypes.h"
namespace Graphics { class Renderer; }

class ModelBase {
public:
	ModelBase() {}
	virtual ~ModelBase() { }
	virtual float GetDrawClipRadius() const = 0;
	virtual void Render(Graphics::Renderer *r, const matrix4x4f &trans, LmrObjParams *params) = 0;
	virtual RefCountedPtr<CollMesh> CreateCollisionMesh(const LmrObjParams *p) = 0;

	virtual void SetLabel(const std::string&) = 0;
};

#endif
