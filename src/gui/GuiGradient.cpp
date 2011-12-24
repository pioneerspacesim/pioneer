#include "Gui.h"
#include "GuiGradient.h"

namespace Gui {

Gradient::Gradient() :
	m_texture(Color(1.0f, 1.0f, 1.0f, 1.0f), Color(0.0f, 0.0f, 0.0f, 1.0f)),
	m_direction(VERTICAL)
{
	SetSize(10.0f, 10.0f);
}

Gradient::Gradient(float width, float height, const Color &begin, const Color &end, Direction direction) :
	m_texture(begin, end),
	m_direction(direction)
{
	SetSize(width, height);
}

void Gradient::GetSizeRequested(float size[2])
{
	GetSize(size);
}

void Gradient::Draw()
{
	float size[2];
	GetSize(size);
	const float w = size[0];
	const float h = size[1];

	//todo: not immediate
	glColor4f(1.f, 0.f, 1.f, 1.f);
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_1D);
	m_texture.Bind();
	glBegin(GL_QUADS);
	if (m_direction == VERTICAL) {
		glTexCoord1f(1.f);
		glVertex2f(0, h);
		glTexCoord1f(1.f);
		glVertex2f(w, h);
		glTexCoord1f(0.f);
		glVertex2f(w, 0);
		glTexCoord1f(0.f);
		glVertex2f(0, 0);
	} else {
		glTexCoord1f(0.f);
		glVertex2f(0, h);
		glTexCoord1f(1.f);
		glVertex2f(w, h);
		glTexCoord1f(1.f);
		glVertex2f(w, 0);
		glTexCoord1f(0.f);
		glVertex2f(0, 0);
	}
	glEnd();
	m_texture.Unbind();
	glDisable(GL_TEXTURE_1D);
	glPopAttrib();
}


Gradient::GradientTexture::GradientTexture(const Color &begin, const Color &end) :
	Texture(GL_TEXTURE_1D, TextureFormat(GL_RGBA, GL_RGBA, GL_FLOAT), CLAMP, LINEAR, false, false),
	m_needGenerate(true)
{
	SetStop(begin, 0.0f);
	SetStop(end, 1.0f);
}

void Gradient::GradientTexture::SetStop(const Color &col, float pos)
{
	pos = Clamp(pos, 0.f, 1.f);
	m_stops[pos] = col;
	m_needGenerate = true;
}

void Gradient::GradientTexture::GenerateGradient()
{
	// XXX this only handles begin & end stops

	const int width = 2;
	assert(m_stops.size() > 0);

	const Color begin = m_stops[0.0f];
	const Color end   = m_stops[1.0f];

	const GLfloat data[width][4] = {
		{ begin.r, begin.g, begin.b, begin.a },
		{ end.r,   end.g,   end.b,   end.a   },
	};

	CreateFromArray(data, width, 0);

	m_needGenerate = false;
}

}
