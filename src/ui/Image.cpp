// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Image.h"
#include "Context.h"
#include "graphics/TextureBuilder.h"

namespace UI {

Image::Image(Context *context, const std::string &filename, Uint32 sizeControlFlags): Widget(context)
	, m_centre(0.0f, 0.0f)
	, m_scale(1.0f)
	, m_preserveAspect(false)
{
	Graphics::TextureBuilder b = Graphics::TextureBuilder::UI(filename);
	m_texture.Reset(b.GetOrCreateTexture(GetContext()->GetRenderer(), "ui"));

	const auto image_size = b.GetDescriptor().GetOriginalSize();
	m_initialSize = Point(image_size.x, image_size.y);

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

Point Image::GetImageSize() const
{
	const auto sz = m_texture->GetDescriptor().GetOriginalSize();
	return Point(sz.x, sz.y);
}

Image *Image::SetHeightLines(Uint32 lines)
{
	const Text::TextureFont *font = GetContext()->GetFont(GetFont()).Get();
	const float height = font->GetHeight() * lines;

	const vector2f sz = m_texture->GetDescriptor().GetOriginalSize();
	const float width = height * sz.x/sz.y;

	m_initialSize = UI::Point(width, height);
	GetContext()->RequestLayout();
	return this;
}

Image *Image::SetNaturalSize()
{
	m_initialSize = GetImageSize();
	GetContext()->RequestLayout();
	return this;
}

void Image::SetTransform(float scale, const vector2f &centre)
{
	m_scale = scale;
	m_centre = centre;
}

void Image::SetPreserveAspect(bool preserve_aspect)
{
	m_preserveAspect = preserve_aspect;
}

void Image::Draw()
{
	const Point &offset = GetActiveOffset();
	const Point &area = GetActiveArea();
	const auto &descriptor = m_texture->GetDescriptor();

	const float half_sx = area.x*0.5f;
	const float half_sy = area.y*0.5f;

	float cx = offset.x + half_sx;
	float cy = offset.y + half_sy;
	float rx, ry;

	if (m_preserveAspect) {
		const vector2f sz = descriptor.GetOriginalSize();
		const float wantRatio = sz.x / sz.y;
		const float haveRatio = float(area.x) / float(area.y);
		if (wantRatio > haveRatio) {
			// limited by width
			rx = half_sx;
			ry = half_sx / wantRatio;
		} else {
			// limited by height
			rx = half_sy * wantRatio;
			ry = half_sy;
		}
	} else {
		rx = half_sx;
		ry = half_sy;
	}

	rx *= m_scale;
	ry *= m_scale;
	cx -= rx*m_centre.x;
	cy -= ry*m_centre.y;
	const vector2f texSize = descriptor.texSize;

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
	va.Add(vector3f(cx-rx, cy-ry, 0.0f), vector2f(0.0f,      0.0f));
	va.Add(vector3f(cx-rx, cy+ry, 0.0f), vector2f(0.0f,      texSize.y));
	va.Add(vector3f(cx+rx, cy-ry, 0.0f), vector2f(texSize.x, 0.0f));
	va.Add(vector3f(cx+rx, cy+ry, 0.0f), vector2f(texSize.x, texSize.y));

	Graphics::Renderer *r = GetContext()->GetRenderer();
	auto renderState = GetContext()->GetSkin().GetAlphaBlendState();
	m_material->diffuse = Color(Color::WHITE.r, Color::WHITE.g, Color::WHITE.b, GetContext()->GetOpacity()*Color::WHITE.a);
	r->DrawTriangles(&va, renderState, m_material.Get(), Graphics::TRIANGLE_STRIP);
}

}
