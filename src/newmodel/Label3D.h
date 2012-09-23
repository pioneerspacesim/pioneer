#ifndef _NEWMODEL_LABEL_H
#define _NEWMODEL_LABEL_H
/*
 * Text geometry node, mostly for ship labels
 */
#include "Node.h"
#include "text/DistanceFieldFont.h"
#include "graphics/Material.h"

namespace Graphics {
	class Renderer;
}

namespace Newmodel {

class Label3D : public Node {
public:
	Label3D(RefCountedPtr<Text::DistanceFieldFont>, Graphics::Renderer*);
	virtual const char *GetTypeName() { return "Label3D"; }
	void SetText(const std::string&);
	virtual void Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd);
	virtual void Accept(NodeVisitor &v);

private:
	RefCountedPtr<Graphics::Material> m_material;
	ScopedPtr<Graphics::VertexArray> m_geometry;
	RefCountedPtr<Text::DistanceFieldFont> m_font;
};

}

#endif
