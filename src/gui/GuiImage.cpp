#include "libs.h"
#include "GuiImage.h"
#include "GuiScreen.h"
#include "GuiTexture.h"

namespace Gui {

Image::Image(const char *filename): Widget(), m_color(Color::WHITE)
{
	Texture *texture = new Texture(filename);
	m_quad.Reset(new Graphics::Drawables::TexturedUIQuad(texture));

	m_width = texture->GetWidth();
	m_height = texture->GetHeight();

	SetSize(m_width, m_height);
}

void Image::GetSizeRequested(float size[2])
{
	size[0] = m_width;
	size[1] = m_height;
}

void Image::Draw()
{
	float allocSize[2];
	GetSize(allocSize);

	Graphics::Renderer *r = Gui::Screen::GetRenderer();
	r->SetBlendMode(Graphics::BLEND_ALPHA);
	m_quad->Draw(r, 0, vector2f(allocSize[0],allocSize[1]), m_color);
}


}

