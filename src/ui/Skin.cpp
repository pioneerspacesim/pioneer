// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Skin.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"

namespace UI {

static const float SKIN_SIZE = 512.0f;

const Skin::BorderedRectElement Skin::s_backgroundNormal(170, 81, 31, 31, 10);
const Skin::BorderedRectElement Skin::s_backgroundActive(170, 206, 31, 31, 10);

const Skin::BorderedRectElement Skin::s_buttonDisabled(10, 12, 35, 35, 12);
const Skin::BorderedRectElement Skin::s_buttonNormal(10, 77, 35, 35, 12);
const Skin::BorderedRectElement Skin::s_buttonHover(10, 142, 35, 35, 12);
const Skin::BorderedRectElement Skin::s_buttonActive(10, 207, 35, 35, 12);

const Skin::RectElement Skin::s_checkboxDisabled(75, 17, 25, 25);
const Skin::RectElement Skin::s_checkboxNormal(75, 87, 25, 25);
const Skin::RectElement Skin::s_checkboxHover(75, 152, 25, 25);
const Skin::RectElement Skin::s_checkboxActive(75, 212, 25, 25);

const Skin::RectElement Skin::s_checkboxCheckedDisabled(120, 17, 25, 25);
const Skin::RectElement Skin::s_checkboxCheckedNormal(120, 87, 25, 25);
const Skin::RectElement Skin::s_checkboxCheckedHover(120, 152, 25, 25);
const Skin::RectElement Skin::s_checkboxCheckedActive(120, 212, 25, 25);

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
	return v * (1.0f / SKIN_SIZE);
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

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);

	va.Add(vector3f(pos.x,              pos.y,       0.0f), scaled(vector2f(element.pos.x,                      element.pos.y)));
	va.Add(vector3f(pos.x,              pos.y+width, 0.0f), scaled(vector2f(element.pos.x,                      element.pos.y+width)));
	va.Add(vector3f(pos.x+width,        pos.y,       0.0f), scaled(vector2f(element.pos.x+width,                element.pos.y)));
	va.Add(vector3f(pos.x+width,        pos.y+width, 0.0f), scaled(vector2f(element.pos.x+width,                element.pos.y+width)));
	va.Add(vector3f(pos.x+size.x-width, pos.y,       0.0f), scaled(vector2f(element.pos.x+element.size.x-width, element.pos.y)));
	va.Add(vector3f(pos.x+size.x-width, pos.y+width, 0.0f), scaled(vector2f(element.pos.x+element.size.x-width, element.pos.y+width)));
	va.Add(vector3f(pos.x+size.x,       pos.y,       0.0f), scaled(vector2f(element.pos.x+element.size.x,       element.pos.y)));
	va.Add(vector3f(pos.x+size.x,       pos.y+width, 0.0f), scaled(vector2f(element.pos.x+element.size.x,       element.pos.y+width)));

	// degenerate triangles to join rows
	va.Add(vector3f(pos.x+size.x,       pos.y+width, 0.0f), scaled(vector2f(element.pos.x+element.size.x,       element.pos.y+width)));
	va.Add(vector3f(pos.x,              pos.y+width,        0.0f), scaled(vector2f(element.pos.x,                      element.pos.y+width)));

	va.Add(vector3f(pos.x,              pos.y+width,        0.0f), scaled(vector2f(element.pos.x,                      element.pos.y+width)));
	va.Add(vector3f(pos.x,              pos.y+size.y-width, 0.0f), scaled(vector2f(element.pos.x,                      element.pos.y+element.size.y-width)));
	va.Add(vector3f(pos.x+width,        pos.y+width,        0.0f), scaled(vector2f(element.pos.x+width,                element.pos.y+width)));
	va.Add(vector3f(pos.x+width,        pos.y+size.y-width, 0.0f), scaled(vector2f(element.pos.x+width,                element.pos.y+element.size.y-width)));
	va.Add(vector3f(pos.x+size.x-width, pos.y+width,        0.0f), scaled(vector2f(element.pos.x+element.size.x-width, element.pos.y+width)));
	va.Add(vector3f(pos.x+size.x-width, pos.y+size.y-width, 0.0f), scaled(vector2f(element.pos.x+element.size.x-width, element.pos.y+element.size.y-width)));
	va.Add(vector3f(pos.x+size.x,       pos.y+width,        0.0f), scaled(vector2f(element.pos.x+element.size.x,       element.pos.y+width)));
	va.Add(vector3f(pos.x+size.x,       pos.y+size.y-width, 0.0f), scaled(vector2f(element.pos.x+element.size.x,       element.pos.y+element.size.y-width)));

	// degenerate triangles to join rows
	va.Add(vector3f(pos.x+size.x,       pos.y+size.y-width, 0.0f), scaled(vector2f(element.pos.x+element.size.x,       element.pos.y+element.size.y-width)));
	va.Add(vector3f(pos.x,              pos.y+size.y-width, 0.0f), scaled(vector2f(element.pos.x,                      element.pos.y+element.size.y-width)));

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
