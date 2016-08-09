// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_BILLBOARD_H
#define _SCENEGRAPH_BILLBOARD_H
/*
 * One or more billboard sprites, meant for lights mostly
 */
#include "Node.h"

namespace Graphics {
	class Material;
	class VertexBuffer;
	class RenderState;
}

namespace SceneGraph {

class Billboard : public Node {
public:
	Billboard(SceneGraph::Model *model, Graphics::Renderer *r, float size);
	Billboard(const Billboard&, NodeCopyCache *cache = 0);
	virtual Node *Clone(NodeCopyCache *cache = 0);
	virtual void Accept(NodeVisitor &v);
	virtual const char *GetTypeName() const { return "Billboard"; }
	virtual void Render(const matrix4x4f &trans, const RenderData *rd);
	void SetColorUVoffset(const vector2f& c) { m_colorUVoffset = c; }

private:
	SceneGraph::Model *m_model;
	float m_size;
	vector2f m_colorUVoffset;
};

}

#endif
