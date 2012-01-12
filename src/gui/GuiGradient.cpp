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

	glEnable(GL_BLEND);
	m_texture.DrawGradientQuad(size[0], size[1], m_direction);
	glDisable(GL_BLEND);
}


Gradient::GradientTexture::GradientTexture(const Color &begin, const Color &end) :
	Texture(GL_TEXTURE_1D, Format(GL_RGBA, GL_RGBA, GL_FLOAT), CLAMP, LINEAR, false),
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

void Gradient::GradientTexture::DrawGradientQuad(float w, float h, Direction direction)
{
	GLfloat vtx[4*2] = {
		0, h,
		w, h,
		w, 0,
		0, 0
	};

	GLfloat vTex[4] = {
		1.0f,
		1.0f,
		0.0f,
		0.0f
	};

	GLfloat hTex[4] = {
		0.0f,
		1.0f,
		1.0f,
		0.0f
	};

	glEnable(GetTarget());
	Bind();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(GLfloat)*2, &vtx[0]);
	glTexCoordPointer(1, GL_FLOAT, sizeof(GLfloat), direction == VERTICAL ? &vTex[0] : &hTex[0]);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	Unbind();
	glDisable(GetTarget());
}

}
