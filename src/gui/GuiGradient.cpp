#include "Gui.h"
#include "GuiGradient.h"

namespace Gui {

Gradient::Gradient(float width, float height, const Color &beginColor, const Color &endColor, Direction direction)
{
	SetSize(width, height);

	const float data[4][4] = {
		{ beginColor.r, beginColor.g, beginColor.b, beginColor.a },
		{ endColor.r,   endColor.g,   endColor.b,   endColor.a   },
	};

	vector2f size = direction == HORIZONTAL ? vector2f(2.0f,1.0f) : vector2f(1.0f,2.0f);
	m_texture.Reset(Gui::Screen::GetRenderer()->CreateTexture(Graphics::TextureDescriptor(Graphics::TEXTURE_RGBA, size)));
	m_texture->Update(data, size, Graphics::IMAGE_RGBA, Graphics::IMAGE_FLOAT);
}

void Gradient::GetSizeRequested(float size[2])
{
	GetSize(size);
}

void Gradient::Draw()
{
	float size[2];
	GetSize(size);

	TexturedQuad(m_texture.Get()).Draw(Gui::Screen::GetRenderer(), vector2f(0.0f), vector2f(size[0],size[1]));
}

}
