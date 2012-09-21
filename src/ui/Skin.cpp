#include "Skin.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"

namespace UI {

static const float SKIN_SIZE = 512.0f;

const Skin::BorderedRectElement Skin::s_backgroundNormal(0, 0, 23, 23, 2);
const Skin::BorderedRectElement Skin::s_backgroundActive(0, 24, 23, 23, 2);

const Skin::BorderedRectElement Skin::s_buttonNormal(24, 0, 23, 23, 4);
const Skin::BorderedRectElement Skin::s_buttonActive(24, 24, 23, 23, 4);

const Skin::RectElement Skin::s_checkboxNormal(48, 0, 23, 23);
const Skin::RectElement Skin::s_checkboxChecked(48, 24, 23, 23);

Skin::Skin(const std::string &filename, Graphics::Renderer *renderer) :
	m_renderer(renderer)
{
	m_texture.Reset(Graphics::TextureBuilder::UI(filename).GetOrCreateTexture(m_renderer, "ui"));

	Graphics::MaterialDescriptor desc;
	desc.textures = 1;
	m_material.Reset(m_renderer->CreateMaterial(desc));
	m_material->texture0 = m_texture.Get();
	m_material->diffuse = Color::WHITE;
}

static inline vector2f scaled(const vector2f &v)
{
	return vector2f(v * 1.0f/SKIN_SIZE);
}

void Skin::DrawRectElement(const RectElement &element, const Point &pos, const Point &size) const
{
	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);

	va.Add(vector3f(pos.x,        pos.y,        0.0f), scaled(vector2f(element.pos.x,                element.pos.y)));
	va.Add(vector3f(pos.x,        pos.y+size.y, 0.0f), scaled(vector2f(element.pos.x,                element.pos.y+element.size.y)));
	va.Add(vector3f(pos.x+size.x, pos.y,        0.0f), scaled(vector2f(element.pos.x+element.size.x, element.pos.y)));
	va.Add(vector3f(pos.x+size.x, pos.y+size.y, 0.0f), scaled(vector2f(element.pos.x+element.size.x, element.pos.y+element.size.y)));

	m_renderer->DrawTriangles(&va, m_material.Get(), Graphics::TRIANGLE_STRIP);
}

void Skin::DrawBorderedRectElement(const BorderedRectElement &element, const Point &pos, const Point &size) const
{
	const float width = element.borderWidth;

	{
	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
	va.Add(vector3f(pos.x,              pos.y,       0.0f), scaled(vector2f(element.pos.x,                      element.pos.y)));
	va.Add(vector3f(pos.x,              pos.y+width, 0.0f), scaled(vector2f(element.pos.x,                      element.pos.y+width)));
	va.Add(vector3f(pos.x+width,        pos.y,       0.0f), scaled(vector2f(element.pos.x+width,                element.pos.y)));
	va.Add(vector3f(pos.x+width,        pos.y+width, 0.0f), scaled(vector2f(element.pos.x+width,                element.pos.y+width)));
	va.Add(vector3f(pos.x+size.x-width, pos.y,       0.0f), scaled(vector2f(element.pos.x+element.size.x-width, element.pos.y)));
	va.Add(vector3f(pos.x+size.x-width, pos.y+width, 0.0f), scaled(vector2f(element.pos.x+element.size.x-width, element.pos.y+width)));
	va.Add(vector3f(pos.x+size.x,       pos.y,       0.0f), scaled(vector2f(element.pos.x+element.size.x,       element.pos.y)));
	va.Add(vector3f(pos.x+size.x,       pos.y+width, 0.0f), scaled(vector2f(element.pos.x+element.size.x,       element.pos.y+width)));
	m_renderer->DrawTriangles(&va, m_material.Get(), Graphics::TRIANGLE_STRIP);
	}

	{
	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
	va.Add(vector3f(pos.x,              pos.y+width,        0.0f), scaled(vector2f(element.pos.x,                      element.pos.y+width)));
	va.Add(vector3f(pos.x,              pos.y+size.y-width, 0.0f), scaled(vector2f(element.pos.x,                      element.pos.y+element.size.y-width)));
	va.Add(vector3f(pos.x+width,        pos.y+width,        0.0f), scaled(vector2f(element.pos.x+width,                element.pos.y+width)));
	va.Add(vector3f(pos.x+width,        pos.y+size.y-width, 0.0f), scaled(vector2f(element.pos.x+width,                element.pos.y+element.size.y-width)));
	va.Add(vector3f(pos.x+size.x-width, pos.y+width,        0.0f), scaled(vector2f(element.pos.x+element.size.x-width, element.pos.y+width)));
	va.Add(vector3f(pos.x+size.x-width, pos.y+size.y-width, 0.0f), scaled(vector2f(element.pos.x+element.size.x-width, element.pos.y+element.size.y-width)));
	va.Add(vector3f(pos.x+size.x,       pos.y+width,        0.0f), scaled(vector2f(element.pos.x+element.size.x,       element.pos.y+width)));
	va.Add(vector3f(pos.x+size.x,       pos.y+size.y-width, 0.0f), scaled(vector2f(element.pos.x+element.size.x,       element.pos.y+element.size.y-width)));
	m_renderer->DrawTriangles(&va, m_material.Get(), Graphics::TRIANGLE_STRIP);
	}

	{
	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
	va.Add(vector3f(pos.x,              pos.y+size.y-width, 0.0f), scaled(vector2f(element.pos.x,                      element.pos.y+element.size.y-width)));
	va.Add(vector3f(pos.x,              pos.y+size.y,       0.0f), scaled(vector2f(element.pos.x,                      element.pos.y+element.size.y)));
	va.Add(vector3f(pos.x+width,        pos.y+size.y-width, 0.0f), scaled(vector2f(element.pos.x+width,                element.pos.y+element.size.y-width)));
	va.Add(vector3f(pos.x+width,        pos.y+size.y,       0.0f), scaled(vector2f(element.pos.x+width,                element.pos.y+element.size.y)));
	va.Add(vector3f(pos.x+size.x-width, pos.y+size.y-width, 0.0f), scaled(vector2f(element.pos.x+element.size.x-width, element.pos.y+element.size.y-width)));
	va.Add(vector3f(pos.x+size.x-width, pos.y+size.y,       0.0f), scaled(vector2f(element.pos.x+element.size.x-width, element.pos.y+element.size.y)));
	va.Add(vector3f(pos.x+size.x,       pos.y+size.y-width, 0.0f), scaled(vector2f(element.pos.x+element.size.x,       element.pos.y+element.size.y-width)));
	va.Add(vector3f(pos.x+size.x,       pos.y+size.y,       0.0f), scaled(vector2f(element.pos.x+element.size.x,       element.pos.y+element.size.y)));
	m_renderer->DrawTriangles(&va, m_material.Get(), Graphics::TRIANGLE_STRIP);
	}
}

}
