// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Image.h"
#include "Context.h"
#include "graphics/TextureBuilder.h"

namespace UI {

Image::Image(Context *context, const std::string &filename, StretchMode stretchMode): Widget(context),
	m_stretchMode(stretchMode)
{
	Graphics::TextureBuilder b = Graphics::TextureBuilder::UI(filename);
	m_texture.Reset(b.GetOrCreateTexture(GetContext()->GetRenderer(), "ui"));

	const Graphics::TextureDescriptor &descriptor = b.GetDescriptor();
	m_initialSize = Point(descriptor.dataSize.x*descriptor.texSize.x,descriptor.dataSize.y*descriptor.texSize.y);

	Graphics::MaterialDescriptor material_desc;
	material_desc.textures = 1;
	m_material.Reset(GetContext()->GetRenderer()->CreateMaterial(material_desc));
	m_material->texture0 = m_texture.Get();
}

Point Image::PreferredSize()
{
	return m_initialSize;
}

void Image::Layout()
{
	Point size = GetSize();

	Point activeArea;

	switch (m_stretchMode) {
		case STRETCH_MAX:
			activeArea = size;
			break;

		case STRETCH_PRESERVE_ASPECT: {

			float originalRatio = float(m_initialSize.x) / float(m_initialSize.y);
			float wantRatio = float(size.x) / float(size.y);

			// more room on X than Y, use full X, scale Y
			if (wantRatio < originalRatio) {
				activeArea.x = size.x;
				activeArea.y = float(size.x) / originalRatio;
			}

			// more room on Y than X, use full Y, scale X
			else {
				activeArea.x = float(size.y) * originalRatio;
				activeArea.y = size.y;
			}

			break;
		}

		default:
			assert(0);
	}

	Point activeOffset(std::max(0, (size.x-activeArea.x)/2), std::max(0, (size.y-activeArea.y)/2));

	SetActiveArea(activeArea, activeOffset);
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
	r->SetBlendMode(Graphics::BLEND_ALPHA);
	r->DrawTriangles(&va, m_material.Get(), Graphics::TRIANGLE_STRIP);
}

}
