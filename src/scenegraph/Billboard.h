// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_BILLBOARD_H
#define _SCENEGRAPH_BILLBOARD_H
/*
 * One or more billboard sprites, meant for lights mostly
 */
#include "Node.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"

namespace SceneGraph {

class Billboard : public Node {
public:
	Billboard(Graphics::Renderer *r, RefCountedPtr<Graphics::Material>, const vector3f &offset, float size);
	Billboard(const Billboard&, NodeCopyCache *cache = 0);
	virtual Node *Clone(NodeCopyCache *cache = 0);
	virtual void Accept(NodeVisitor &v);
	virtual const char *GetTypeName() const { return "Billboard"; }
	virtual void Render(const matrix4x4f &trans, const RenderData *rd);
	void SetMaterial(RefCountedPtr<Graphics::Material> mat) { m_material = mat; }

private:
	float m_size;
	RefCountedPtr<Graphics::Material> m_material;
	Graphics::RenderState *m_renderState;
	vector3f m_offset;
};

}

#endif
