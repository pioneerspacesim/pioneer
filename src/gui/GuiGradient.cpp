#include "Gui.h"
#include "GuiGradient.h"

namespace Gui {

Gradient::Gradient(float width, float height, const Color &begin, const Color &end, Direction direction)
{
	SetSize(width, height);
	m_quad.Reset(new TexturedQuad(new GradientTexture(begin, end, direction)));
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


Gradient::GradientTexture::GradientTexture(const Color &begin, const Color &end, Direction direction) :
	Texture(Texture::TARGET_2D, Format(Texture::Format::INTERNAL_RGBA, Texture::Format::DATA_RGBA, Texture::Format::DATA_FLOAT), CLAMP, LINEAR, false, true)
{
	const float data[4][4] = {
		{ begin.r, begin.g, begin.b, begin.a },
		{ end.r,   end.g,   end.b,   end.a   },
	};

	if (direction == HORIZONTAL)
		CreateFromArray(data, 2, 1);
	else
		CreateFromArray(data, 1, 2);
}

}
