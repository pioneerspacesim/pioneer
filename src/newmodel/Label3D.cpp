#include "newmodel/Label3D.h"

namespace Newmodel {

Label3D::Label3D(RefCountedPtr<Text::TextureFont> font)
: Node(NODE_TRANSPARENT)
, m_font(font)
{
	m_geometry.Reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0));
	m_material.Reset(new Graphics::Material);
	m_material->texture0 = m_font->GetTexture().Get();
	m_material->twoSided = true;
	m_material->diffuse = Color::GREEN;
}

void Label3D::SetText(const std::string &text)
{
	//regenerate geometry
	m_geometry->Clear();
	m_font->CreateGeometry(*m_geometry.Get(), text.c_str(), 0.f, 0.f, Color::RED);
}

void Label3D::Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd)
{
	//needs alpha test!
	r->SetTransform(trans);
	r->DrawTriangles(m_geometry.Get(), m_material.Get());
}

}