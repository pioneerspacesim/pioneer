// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_SHIELD_H
#define _SCENEGRAPH_SHIELD_H
/*
 * Spaceship shield
 */
#include "libs.h"
#include "Node.h"
#include "StaticGeometry.h"

namespace Graphics {
	class Renderer;
	class VertexArray;
	class Material;
}

namespace SceneGraph {

class Shield : public StaticGeometry {
public:
	Shield(Graphics::Renderer *);
	Shield(const Shield&, NodeCopyCache *cache = 0);
	Node *Clone(NodeCopyCache *cache = 0);
	virtual void Accept(NodeVisitor &v);
	virtual const char *GetTypeName() { return "Shield"; }
	virtual void Render(const matrix4x4f &trans, RenderData *rd);

private:
	bool visible;
};

}

#endif
