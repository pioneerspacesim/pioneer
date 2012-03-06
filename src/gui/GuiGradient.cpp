#include "Gui.h"
#include "GuiGradient.h"

namespace Gui {

Gradient::Gradient(float width, float height, const Color &begin, const Color &end, Direction direction)
{
	SetSize(width, height);
	m_quad.Reset(new TexturedQuad(Gui::Screen::GetRenderer()->GetTexture(GradientTextureDescriptor(begin, end, direction))));
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


Gradient::GradientTextureDescriptor::GradientTextureDescriptor(const Color &_beginColor, const Color &_endColor, Direction _direction) :
    Graphics::TextureDescriptor(TARGET_2D, Format(Format::INTERNAL_RGBA, Format::DATA_RGBA, Format::DATA_FLOAT), Options(Options::CLAMP, Options::LINEAR, false)),
	beginColor(_beginColor), endColor(_endColor), direction(_direction)
{
}

const Graphics::TextureDescriptor::Data *Gradient::GradientTextureDescriptor::GetData() const {
	const float data[4][4] = {
		{ beginColor.r, beginColor.g, beginColor.b, beginColor.a },
		{ endColor.r,   endColor.g,   endColor.b,   endColor.a   },
	};

	if (direction == HORIZONTAL)
		return new Graphics::TextureDescriptor::Data(data, vector2f(2.0f,1.0f));
	else
		return new Graphics::TextureDescriptor::Data(data, vector2f(1.0f,2.0f));
}

}
