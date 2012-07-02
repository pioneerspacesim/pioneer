#include "Label3D.h"
#include "NodeVisitor.h"

namespace Newmodel {

Label3D::Label3D(RefCountedPtr<Text::DistanceFieldFont> font)
: Node(NODE_TRANSPARENT)
, m_font(font)
{
	m_geometry.Reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0));
	m_material.Reset(new Graphics::Material);
	m_material->texture0 = font->GetTexture();
	m_material->twoSided = false;
	m_material->diffuse = Color::WHITE;
}

void Label3D::SetText(const std::string &text)
{
	//regenerate geometry
	m_geometry->Clear();
	m_font->GetGeometry(*m_geometry.Get(), text, vector2f(0.f));
}

void Label3D::Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd)
{
	//needs alpha test!
	r->SetTransform(trans);
	r->DrawTriangles(m_geometry.Get(), m_material.Get());
}

void Label3D::Accept(NodeVisitor &nv)
{
	nv.ApplyLabel(*this);
}

}