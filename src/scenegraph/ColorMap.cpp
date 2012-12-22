// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ColorMap.h"
#include "graphics/Renderer.h"

namespace SceneGraph {

ColorMap::ColorMap()
: m_smooth(true)
{

}

Graphics::Texture *ColorMap::GetTexture()
{
	assert(m_texture.Valid());
	return m_texture.Get();
}

void ColorMap::AddColor(int width, const Color4ub &c, std::vector<unsigned char> &out)
{
	for (int i=0; i < width; i++) {
		out.push_back(c.r);
		out.push_back(c.g);
		out.push_back(c.b);
	}
}

void ColorMap::Generate(Graphics::Renderer *r, const Color4ub &a, const Color4ub &b, const Color4ub &c)
{
	std::vector<unsigned char> colors;
	const int w = 4;
	AddColor(w, Color4ub(255, 255, 255), colors);
	AddColor(w, a, colors);
	AddColor(w, b, colors);
	AddColor(w, c, colors);
	vector2f size(colors.size()/3, 1.f);
	const Graphics::TextureSampleMode sampleMode = m_smooth ? Graphics::LINEAR_CLAMP : Graphics::NEAREST_CLAMP;
	Graphics::Texture *texture = r->CreateTexture(Graphics::TextureDescriptor(Graphics::TEXTURE_RGB, size, sampleMode));
	if (!m_texture.Valid()) m_texture.Reset(texture);
	m_texture->Update(&colors[0], size, Graphics::IMAGE_RGB, Graphics::IMAGE_UNSIGNED_BYTE);
}

void ColorMap::SetSmooth(bool smooth)
{
	m_smooth = smooth;
	if (m_texture.Valid()) {
		m_texture->SetSampleMode(m_smooth ? Graphics::LINEAR_CLAMP : Graphics::NEAREST_CLAMP);
	}
}

}
