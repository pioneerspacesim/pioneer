#include "Skin.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"

namespace UI {

static const float SKIN_SIZE = 512.0f;

const Skin::BorderedRectElement Skin::s_buttonNormal(0.0f, 0.0f, 23.0f, 23.0f, 4.0f);
const Skin::BorderedRectElement Skin::s_buttonActive(0.0f, 24.0f, 23.0f, 23.0f, 4.0f);

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

static void AddRectToVertexArray(Graphics::VertexArray &va, const vector2f &pos, const vector2f &size, const vector2f &texPos, const vector2f &texSize)
{
	const vector2f scaledTexPos(texPos * 1.0f/SKIN_SIZE);
	const vector2f scaledTexSize(texSize * 1.0f/SKIN_SIZE);

	va.Add(vector3f(pos.x,        pos.y,        0.0f), vector2f(scaledTexPos.x,                 scaledTexPos.y));
	va.Add(vector3f(pos.x,        pos.y+size.y, 0.0f), vector2f(scaledTexPos.x,                 scaledTexPos.y+scaledTexSize.y));
	va.Add(vector3f(pos.x+size.x, pos.y,        0.0f), vector2f(scaledTexPos.x+scaledTexSize.x, scaledTexPos.y));
	va.Add(vector3f(pos.x+size.x, pos.y+size.y, 0.0f), vector2f(scaledTexPos.x+scaledTexSize.x, scaledTexPos.y+scaledTexSize.y));
}

void Skin::DrawRectElement(const RectElement &element, const vector2f &pos, const vector2f &size) const
{
	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
	AddRectToVertexArray(va, pos, size, element.pos, element.size);
	m_renderer->DrawTriangles(&va, m_material.Get(), Graphics::TRIANGLE_STRIP);
}

void Skin::DrawBorderedRectElement(const BorderedRectElement &element, const vector2f &pos, const vector2f &size) const
{
	// XXX this actually does nothing with borders
	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
	AddRectToVertexArray(va, pos, size, element.pos, element.size);
	m_renderer->DrawTriangles(&va, m_material.Get(), Graphics::TRIANGLE_STRIP);
}

}
