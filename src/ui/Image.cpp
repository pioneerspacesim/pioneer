// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Image.h"
#include "Context.h"
#include "graphics/TextureBuilder.h"

namespace UI {

Image::Image(Context *context, const std::string &filename, Uint32 sizeControlFlags): Widget(context)
{
	Graphics::TextureBuilder b = Graphics::TextureBuilder::UI(filename);
	m_texture.Reset(b.GetOrCreateTexture(GetContext()->GetRenderer(), "ui"));

	const Graphics::TextureDescriptor &descriptor = b.GetDescriptor();
	m_initialSize = Point(descriptor.dataSize.x*descriptor.texSize.x,descriptor.dataSize.y*descriptor.texSize.y);

	Graphics::MaterialDescriptor material_desc;
	material_desc.textures = 1;
	m_material.Reset(GetContext()->GetRenderer()->CreateMaterial(material_desc));
	m_material->texture0 = m_texture.Get();

	SetSizeControlFlags(sizeControlFlags);
}

Point Image::PreferredSize()
{
	return m_initialSize;
}

Image *Image::SetHeightLines(Uint32 lines)
{
	const Text::TextureFont *font = GetContext()->GetFont(GetFont()).Get();
	const float height = font->GetHeight() * lines;
	m_initialSize = UI::Point(height * float(m_initialSize.x) / float(m_initialSize.y), height);
	GetContext()->RequestLayout();
	return this;
}

void Image::Draw()
{
	const Point &offset = GetActiveOffset();
	const Point &area = GetActiveArea();

	const float x = offset.x;
	const float y = offset.y;
	const float sx = area.x;
	const float sy = area.y;

	const vector2f texSize = m_texture->GetDescriptor().texSize;

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
	va.Add(vector3f(x,    y,    0.0f), vector2f(0.0f,      0.0f));
	va.Add(vector3f(x,    y+sy, 0.0f), vector2f(0.0f,      texSize.y));
	va.Add(vector3f(x+sx, y,    0.0f), vector2f(texSize.x, 0.0f));
	va.Add(vector3f(x+sx, y+sy, 0.0f), vector2f(texSize.x, texSize.y));

	Graphics::Renderer *r = GetContext()->GetRenderer();
	auto renderState = GetContext()->GetSkin().GetAlphaBlendState();
	r->DrawTriangles(&va, renderState, m_material.Get(), Graphics::TRIANGLE_STRIP);
}

}
