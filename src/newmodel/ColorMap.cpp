#include "ColorMap.h"

namespace Newmodel {

Graphics::Texture *ColorMap::GetTexture()
{
	assert(m_texture.Valid());
	return m_texture.Get();
}

void ColorMap::Generate(Graphics::Renderer *r, const Color4ub &a, const Color4ub &b, const Color4ub &c)
{
	const unsigned char data[4*3] = {
		255, 255, 255, //white
		a.r, a.g, a.b,
		b.r, b.g, b.b,
		c.r, c.g, c.b
	};
	vector2f size(4,1);
	Graphics::Texture *texture = r->CreateTexture(Graphics::TextureDescriptor(Graphics::TEXTURE_RGB, size));
	texture->Update(data, size, Graphics::IMAGE_RGB, Graphics::IMAGE_UNSIGNED_BYTE);
	m_texture.Reset(texture);
}

}