// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Label3D.h"
#include "NodeVisitor.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"

namespace SceneGraph {

Label3D::Label3D(RefCountedPtr<Text::DistanceFieldFont> font, Graphics::Renderer *r)
: Node(NODE_SOLID) //appropriate for alpha testing
, m_font(font)
{
	Graphics::MaterialDescriptor matdesc;
	matdesc.textures = 1;
	matdesc.alphaTest = true;
	matdesc.lighting = true;
	m_geometry.Reset(font->CreateVertexArray());
	m_material.Reset(r->CreateMaterial(matdesc));
	m_material->texture0 = font->GetTexture();
	m_material->diffuse = Color::WHITE;
	m_material->emissive = Color(0.15f);
	m_material->specular = Color::WHITE;
}

void Label3D::SetText(const std::string &text)
{
	//regenerate geometry
	m_geometry->Clear();
	m_font->GetGeometry(*m_geometry.Get(), text, vector2f(0.f));
}

void Label3D::Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd)
{
	//needs alpha test
	r->SetTransform(trans);
	r->DrawTriangles(m_geometry.Get(), m_material.Get());
}

void Label3D::Accept(NodeVisitor &nv)
{
	nv.ApplyLabel(*this);
}

}
