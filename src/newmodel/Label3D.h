#ifndef _NEWMODEL_LABEL_H
#define _NEWMODEL_LABEL_H
/*
 * Text geometry node, mostly for ship labels
 */
#include "Node.h"
#include "text/TextureFont.h"

namespace Newmodel {

class Label3D : public Node {
public:
	Label3D(RefCountedPtr<Text::TextureFont> font);
	void SetText(const std::string&);
	virtual void Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd);

private:
	RefCountedPtr<Text::TextureFont> m_font;
	RefCountedPtr<Graphics::Material> m_material;
	ScopedPtr<Graphics::VertexArray> m_geometry;
};

}

#endif
