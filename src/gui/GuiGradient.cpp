// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gui.h"
#include "GuiGradient.h"

namespace Gui {

Gradient::Gradient(float width, float height, const Color &beginColor, const Color &endColor, Direction direction)
{
	SetSize(width, height);

	const Uint8 data[4][4] = {
		{ beginColor.r, beginColor.g, beginColor.b, beginColor.a },
		{ endColor.r, endColor.g, endColor.b, endColor.a },
	};

	const Graphics::TextureFormat format = Graphics::TEXTURE_RGBA_8888;

	vector2f size = direction == HORIZONTAL ? vector2f(2.0f,1.0f) : vector2f(1.0f,2.0f);
	Graphics::Texture *texture = Gui::Screen::GetRenderer()->CreateTexture(Graphics::TextureDescriptor(format, size));
	texture->Update(data, size, format);
	m_quad.reset(new TexturedQuad(texture));
}

void Gradient::GetSizeRequested(float size[2])
{
	GetSize(size);
}

void Gradient::Draw()
{
	PROFILE_SCOPED()
	float size[2];
	GetSize(size);

	m_quad->Draw(Gui::Screen::GetRenderer(), vector2f(0.0f), vector2f(size[0],size[1]));
}

}
