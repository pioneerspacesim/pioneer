#include "libs.h"
#include "GuiImage.h"
#include "GuiScreen.h"
#include "Texture.h"

namespace Gui {

Image::Image(const char *filename): Widget()
{
	m_texture = Gui::Screen::GetTextureCache()->GetUITexture(filename);

	SetSize(float(m_texture->GetWidth()), float(m_texture->GetHeight()));

	m_col[0] = m_col[1] = m_col[2] = m_col[3] = 1.0f;
}

void Image::GetSizeRequested(float size[2])
{
	size[0] = float(m_texture->GetWidth());
	size[1] = float(m_texture->GetHeight());
}

void Image::SetModulateColor(float r, float g, float b, float a)
{
	m_col[0] = r;
	m_col[1] = g;
	m_col[2] = b;
	m_col[3] = a;
}

void Image::Draw()
{
	float allocSize[2];
	GetSize(allocSize);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	if ((m_col[0] >= 1) && (m_col[1] >= 1) && (m_col[2] >= 1) && (m_col[3] >= 1)) {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	} else {
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor4fv(m_col);
	}

	m_texture->DrawUIQuad(allocSize[0], allocSize[1]);

	glDisable(GL_BLEND);
}


}

