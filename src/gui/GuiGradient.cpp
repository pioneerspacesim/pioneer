#include "Gui.h"
#include "GuiGradient.h"

namespace Gui {

Gradient::Gradient(float width, float height, const Color &beginColor, const Color &endColor, Direction direction)
{
	SetSize(width, height);

	Color4ub c0(beginColor);
	Color4ub c1(endColor);
	const unsigned char data[4][4] = {
		{ c0.r, c0.g, c0.b, c0.a },
		{ c1.r, c1.g, c1.b, c1.a },
	};

	vector2f size = direction == HORIZONTAL ? vector2f(2.0f,1.0f) : vector2f(1.0f,2.0f);
	Graphics::Texture *texture = Gui::Screen::GetRenderer()->CreateTexture(Graphics::TextureDescriptor(Graphics::TEXTURE_RGBA, size));
	texture->Update(data, size, Graphics::IMAGE_RGBA, Graphics::IMAGE_UNSIGNED_BYTE);
	m_quad.Reset(new TexturedQuad(texture));
}

void Gradient::GetSizeRequested(float size[2])
{
	GetSize(size);
}

void Gradient::Draw()
{
	float size[2];
	GetSize(size);

	m_quad->Draw(Gui::Screen::GetRenderer(), vector2f(0.0f), vector2f(size[0],size[1]));
}

}
