// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_BILLBOARD_H
#define _SCENEGRAPH_BILLBOARD_H
/*
 * One or more billboard sprites, meant for lights mostly
 */
#include "Node.h"
#include "graphics/Material.h"

namespace SceneGraph {

class Billboard : public Node {
public:
	Billboard(Graphics::Renderer *r, RefCountedPtr<Graphics::Material>, const vector3f &offset, float size);
	Billboard(const Billboard&);
	virtual Node *Clone();
	virtual void Accept(NodeVisitor &v);
	virtual const char *GetTypeName() { return "Billboard"; }
	virtual void Render(const matrix4x4f &trans, RenderData *rd);

private:
	float m_size;
	RefCountedPtr<Graphics::Material> m_material;
	vector3f m_offset;
};

}

#endif
