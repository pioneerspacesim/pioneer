#include "Skin.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"

namespace UI {

static const float SKIN_SIZE = 512.0f;

Skin::BorderedRectElement Skin::s_buttonNormal(0.0f, 0.0f, 23.0f, 23.0f, 4.0f);
Skin::BorderedRectElement Skin::s_buttonActive(0.0f, 24.0f, 23.0f, 23.0f, 4.0f);

Skin::RectElement::RectElement(float x, float y, float w, float h) : pos(vector2f(x,y) * 1.0f/SKIN_SIZE), size(vector2f(w,h) * 1.0f/SKIN_SIZE) {}

Skin::Skin(const std::string &filename, Graphics::Renderer *renderer) :
	m_renderer(renderer)
{
	m_texture.Reset(Graphics::TextureBuilder::UI(filename).GetOrCreateTexture(m_renderer, "ui"));

	m_material.Reset(new Graphics::Material);
	m_material->unlit = true;
	m_material->texture0 = m_texture.Get();
	m_material->vertexColors = false;
	m_material->diffuse = Color::WHITE;
}

void Skin::DrawBorderedRectElement(const BorderedRectElement &element, const vector2f &pos, const vector2f &size) const
{
	// XXX this actually does nothing with borders

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);

	va.Add(vector3f(pos.x,        pos.y,        0.0f), vector2f(element.pos.x,                element.pos.y));
	va.Add(vector3f(pos.x,        pos.y+size.y, 0.0f), vector2f(element.pos.x,                element.pos.y+element.size.y));
	va.Add(vector3f(pos.x+size.x, pos.y,        0.0f), vector2f(element.pos.x+element.size.x, element.pos.y));
	va.Add(vector3f(pos.x+size.x, pos.y+size.y, 0.0f), vector2f(element.pos.x+element.size.x, element.pos.y+element.size.y));

	m_renderer->DrawTriangles(&va, m_material.Get(), Graphics::TRIANGLE_STRIP);
}

}
