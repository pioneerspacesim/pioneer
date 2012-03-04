#include "Gui.h"
#include "GuiGradient.h"

namespace Gui {

Gradient::Gradient(float width, float height, const Color &begin, const Color &end, Direction direction)
{
	SetSize(width, height);
	m_quad.Reset(new TexturedQuad(new GradientTexture(Gui::Screen::GetRenderer(), begin, end, direction)));
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


Gradient::GradientTexture::GradientTexture(Graphics::Renderer *r, const Color &begin, const Color &end, Direction direction) :
    Graphics::Texture(TARGET_2D, Format(Format::INTERNAL_RGBA, Format::DATA_RGBA, Format::DATA_FLOAT), Options(Options::CLAMP, Options::LINEAR, false))
{
	const float data[4][4] = {
		{ begin.r, begin.g, begin.b, begin.a },
		{ end.r,   end.g,   end.b,   end.a   },
	};

	if (direction == HORIZONTAL)
		CreateFromArray(r, data, 2, 1);
	else
		CreateFromArray(r, data, 1, 2);
}

}
