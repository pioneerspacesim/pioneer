// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_LABEL_H
#define _SCENEGRAPH_LABEL_H
/*
 * Text geometry node, mostly for ship labels
 */
#include "Node.h"
#include "text/DistanceFieldFont.h"
#include "graphics/Material.h"

namespace Graphics {
	class Renderer;
}

namespace SceneGraph {

class Label3D : public Node {
public:
	Label3D(Graphics::Renderer *r, RefCountedPtr<Text::DistanceFieldFont>);
	Label3D(const Label3D&, NodeCopyCache *cache = 0);
	virtual Node *Clone(NodeCopyCache *cache = 0);
	virtual const char *GetTypeName() const { return "Label3D"; }
	void SetText(const std::string&);
	virtual void Render(const matrix4x4f &trans, const RenderData *rd);
	virtual void Accept(NodeVisitor &v);

private:
	RefCountedPtr<Graphics::Material> m_material;
	std::unique_ptr<Graphics::VertexArray> m_geometry;
	RefCountedPtr<Text::DistanceFieldFont> m_font;
};

}

#endif
