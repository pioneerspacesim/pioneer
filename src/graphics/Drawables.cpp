#include "Drawables.h"
#include "Material.h"

namespace Graphics {

namespace Drawables {

Line3D::Line3D()
{
	m_points[0] = vector3f(0.f);
	m_points[1] = vector3f(0.f);
	m_colors[0] = Color(0.f);
	m_colors[1] = Color(1.f);
	m_width     = 3.f;
}

void Line3D::SetStart(const vector3f &s)
{
	m_points[0] = s;
}

void Line3D::SetEnd(const vector3f &e)
{
	m_points[1] = e;
}

void Line3D::SetColor(const Color &c)
{
	m_colors[0]  = c;
	m_colors[1]  = c;
	m_colors[1]  *= 0.5; //XXX hardcoded appearance
}

void Line3D::Draw(Renderer *renderer)
{
	// XXX would be nicer to draw this as a textured triangle strip
	// can't guarantee linewidth support
	glLineWidth(m_width);
	renderer->DrawLines(2, m_points, m_colors);
	glLineWidth(1.f);
}

void TexturedUIQuad::Draw(Renderer *renderer)
{
	Draw(renderer, 0.0f, 1.0f);
}

void TexturedUIQuad::Draw(Renderer *renderer, const vector2f &pos, const vector2f &size, const vector2f &texPos, const vector2f &texSize, const Color &tint)
{
    VertexArray va(ATTRIB_POSITION | ATTRIB_UV0);

	va.Add(vector3f(pos.x,        pos.y,        0.0f), vector2f(texPos.x,           texPos.y));
	va.Add(vector3f(pos.x,        pos.y+size.y, 0.0f), vector2f(texPos.x,           texPos.y+texSize.y));
	va.Add(vector3f(pos.x+size.x, pos.y,        0.0f), vector2f(texPos.x+texSize.x, texPos.y));
	va.Add(vector3f(pos.x+size.x, pos.y+size.y, 0.0f), vector2f(texPos.x+texSize.x, texPos.y+texSize.y));

	Draw(renderer, &va, tint);
}

void TexturedUIQuad::Draw(Renderer *renderer, VertexArray *va, const Color &tint)
{
	Material m;
	m.unlit = true;
	m.texture0 = m_texture.Get();
	m.vertexColors = false;
	m.diffuse = tint;
	renderer->SetBlendMode(Graphics::BLEND_ALPHA);
	renderer->DrawTriangles(va, &m, TRIANGLE_STRIP);
}

}

}
