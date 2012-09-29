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
	m_quad.Reset(new Gui::TexturedQuad(b.GetOrCreateTexture(GetContext()->GetRenderer(), "ui")));

	const Graphics::TextureDescriptor &descriptor = b.GetDescriptor();
	m_initialSize = Point(descriptor.dataSize.x*descriptor.texSize.x,descriptor.dataSize.y*descriptor.texSize.y);
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

		case STRETCH_PRESERVE: {

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

	Graphics::Renderer *r = GetContext()->GetRenderer();
	r->SetBlendMode(Graphics::BLEND_ALPHA);
	m_quad->Draw(r, vector2f(offset.x, offset.y), vector2f(area.x, area.y));
}

}
