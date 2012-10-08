// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Skin.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"

namespace UI {

static const float SKIN_SIZE = 512.0f;

Skin::BorderedRectElement Skin::s_backgroundNormal;
Skin::BorderedRectElement Skin::s_backgroundActive;

Skin::BorderedRectElement Skin::s_buttonDisabled;
Skin::BorderedRectElement Skin::s_buttonNormal;
Skin::BorderedRectElement Skin::s_buttonHover;
Skin::BorderedRectElement Skin::s_buttonActive;

Skin::RectElement Skin::s_checkboxDisabled;
Skin::RectElement Skin::s_checkboxNormal;
Skin::RectElement Skin::s_checkboxHover;
Skin::RectElement Skin::s_checkboxActive;

Skin::RectElement Skin::s_checkboxCheckedDisabled;
Skin::RectElement Skin::s_checkboxCheckedNormal;
Skin::RectElement Skin::s_checkboxCheckedHover;
Skin::RectElement Skin::s_checkboxCheckedActive;

Skin::Skin(const std::string &filename, Graphics::Renderer *renderer) :
	m_config(filename),
	m_renderer(renderer)
{
	m_texture.Reset(Graphics::TextureBuilder::UI(m_config.String("TextureFile")).GetOrCreateTexture(m_renderer, "ui"));

	Graphics::MaterialDescriptor desc;
	desc.textures = 1;
	m_material.Reset(m_renderer->CreateMaterial(desc));
	m_material->texture0 = m_texture.Get();
	m_material->diffuse = Color::WHITE;

	s_backgroundNormal        = LoadBorderedRectElement(m_config.String("BackgroundNormal"));
	s_backgroundActive        = LoadBorderedRectElement(m_config.String("BackgroundActive"));
	s_buttonDisabled          = LoadBorderedRectElement(m_config.String("ButtonDisabled"));
	s_buttonNormal            = LoadBorderedRectElement(m_config.String("ButtonNormal"));
	s_buttonHover             = LoadBorderedRectElement(m_config.String("ButtonHover"));
	s_buttonActive            = LoadBorderedRectElement(m_config.String("ButtonActive"));
	s_checkboxDisabled        = LoadRectElement(m_config.String("CheckboxDisabled"));
	s_checkboxNormal          = LoadRectElement(m_config.String("CheckboxNormal"));
	s_checkboxHover           = LoadRectElement(m_config.String("CheckboxHover"));
	s_checkboxActive          = LoadRectElement(m_config.String("CheckboxActive"));
	s_checkboxCheckedDisabled = LoadRectElement(m_config.String("CheckboxCheckedDisabled"));
	s_checkboxCheckedNormal   = LoadRectElement(m_config.String("CheckboxCheckedNormal"));
	s_checkboxCheckedHover    = LoadRectElement(m_config.String("CheckboxCheckedHover"));
	s_checkboxCheckedActive   = LoadRectElement(m_config.String("CheckboxCheckedActive"));
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

static void SplitSpec(const std::string &spec, std::vector<int> &output)
{
	static const std::string delim(",");

	size_t i = 0, start = 0, end = 0;
	while (end != std::string::npos) {
		// get to the first non-delim char
		start = spec.find_first_not_of(delim, end);

		// read the end, no more to do
		if (start == std::string::npos)
			break;

		// find the end - next delim or end of string
		end = spec.find_first_of(delim, start);

		// extract the fragment and remember it
		output[i++] = atoi(spec.substr(start, (end == std::string::npos) ? std::string::npos : end - start).c_str());
	}
}

Skin::RectElement Skin::LoadRectElement(const std::string &spec)
{
	std::vector<int> v(4);
	SplitSpec(spec, v);
	return RectElement(v[0], v[1], v[2], v[3]);
}

Skin::BorderedRectElement Skin::LoadBorderedRectElement(const std::string &spec)
{
	std::vector<int> v(5);
	SplitSpec(spec, v);
	return BorderedRectElement(v[0], v[1], v[2], v[3], v[4]);
}

Skin::Config::Config(const std::string &filename) : IniConfig(filename)
{
	Load();
}

}
