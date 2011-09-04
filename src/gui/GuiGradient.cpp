#include "Gui.h"
#include "GuiGradient.h"

namespace Gui {

Gradient::Gradient() :
	m_direction(HORIZONTAL),
	m_texture(0)
{
	SetSize(10.f, 10.f);
	SetStop(Color(1.f,1.f,1.f,1.f), 0.f);
	SetStop(Color(0.f,0.f,1.f,1.f), 1.f);
}

Gradient::Gradient(float w, float h, const Color &begin, const Color &end, Direction dir) :
	m_direction(dir),
	m_texture(0)
{
	SetSize(w, h);
	SetStop(begin, 0.f);
	SetStop(end, 1.f);
}

Gradient::~Gradient()
{
	glDeleteTextures(1, &m_texture);
}

void Gradient::GetSizeRequested(float size[2])
{
	GetSize(size);
}

void Gradient::SetStop(const Color &col, float pos)
{
	pos = Clamp(pos, 0.f, 1.f);
	m_stops[pos] = col;
	if (m_texture > 0) {
		glDeleteTextures(1, &m_texture);
		m_texture = 0;
	}
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
	glBindTexture(GL_TEXTURE_1D, GetTexture());
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
	glPopAttrib();
}

//Get the current texture or regenerate if updated
GLuint Gradient::GetTexture()
{
	if (m_texture == 0)
		m_texture = GenerateTexture();

	return m_texture;
}

//Generate a new texture
//Remember to delete if you use this elsewhere
GLuint Gradient::GenerateTexture()
{
	const int width = 2;
	assert(m_stops.size() > 0);
	GLuint tex;

	const Color beg = m_stops[0.f];
	const Color end = m_stops[1.f];

	const GLfloat buf[width][4] = {
		{ beg.r, beg.g, beg.b, beg.a },
		{ end.r, end.g, end.b, end.a },
	};

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_1D, tex);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, width, 0,
		GL_RGBA, GL_FLOAT, buf);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_1D, 0);
	return tex;
}

}
