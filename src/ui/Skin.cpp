// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Skin.h"
#include "IniConfig.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"
#include "FileSystem.h"

namespace UI {

static const float SKIN_SIZE = 512.0f;

Skin::Skin(const std::string &filename, Graphics::Renderer *renderer, float scale) :
	m_renderer(renderer),
	m_scale(scale)
{
	IniConfig cfg;
	// set defaults
	cfg.SetInt("ButtonMinInnerSize", 16);
	cfg.SetFloat("ListAlphaNormal", 0.0);
	cfg.SetFloat("ListAlphaHover", 0.4);
	cfg.SetFloat("ListAlphaSelect", 0.6);
	// load
	cfg.Read(FileSystem::gameDataFiles, filename);

	m_texture.Reset(Graphics::TextureBuilder::UI(cfg.String("TextureFile")).GetOrCreateTexture(m_renderer, "ui"));

	Graphics::MaterialDescriptor desc;
	desc.textures = 1;
	m_material.Reset(m_renderer->CreateMaterial(desc));
	m_material->texture0 = m_texture.Get();
	m_material->diffuse = Color::WHITE;

	m_backgroundNormal        = LoadBorderedRectElement(cfg.String("BackgroundNormal"));
	m_backgroundActive        = LoadBorderedRectElement(cfg.String("BackgroundActive"));

	m_buttonDisabled          = LoadBorderedRectElement(cfg.String("ButtonDisabled"));
	m_buttonNormal            = LoadBorderedRectElement(cfg.String("ButtonNormal"));
	m_buttonHover             = LoadBorderedRectElement(cfg.String("ButtonHover"));
	m_buttonActive            = LoadBorderedRectElement(cfg.String("ButtonActive"));

	m_checkboxDisabled        = LoadRectElement(cfg.String("CheckboxDisabled"));
	m_checkboxNormal          = LoadRectElement(cfg.String("CheckboxNormal"));
	m_checkboxHover           = LoadRectElement(cfg.String("CheckboxHover"));
	m_checkboxActive          = LoadRectElement(cfg.String("CheckboxActive"));
	m_checkboxCheckedDisabled = LoadRectElement(cfg.String("CheckboxCheckedDisabled"));
	m_checkboxCheckedNormal   = LoadRectElement(cfg.String("CheckboxCheckedNormal"));
	m_checkboxCheckedHover    = LoadRectElement(cfg.String("CheckboxCheckedHover"));
	m_checkboxCheckedActive   = LoadRectElement(cfg.String("CheckboxCheckedActive"));

	m_sliderVerticalGutter         = LoadEdgedRectElement(cfg.String("SliderVerticalGutter"));
	m_sliderHorizontalGutter       = LoadEdgedRectElement(cfg.String("SliderHorizontalGutter"));
	m_sliderVerticalButtonNormal   = LoadRectElement(cfg.String("SliderVerticalButtonNormal"));
	m_sliderVerticalButtonHover    = LoadRectElement(cfg.String("SliderVerticalButtonHover"));
	m_sliderVerticalButtonActive   = LoadRectElement(cfg.String("SliderVerticalButtonActive"));
	m_sliderHorizontalButtonNormal = LoadRectElement(cfg.String("SliderHorizontalButtonNormal"));
	m_sliderHorizontalButtonHover  = LoadRectElement(cfg.String("SliderHorizontalButtonHover"));
	m_sliderHorizontalButtonActive = LoadRectElement(cfg.String("SliderHorizontalButtonActive"));

	m_buttonMinInnerSize      = cfg.Int("ButtonMinInnerSize");

	m_listAlphaNormal = cfg.Float("ListAlphaNormal");
	m_listAlphaSelect = cfg.Float("ListAlphaSelect");
	m_listAlphaHover  = cfg.Float("ListAlphaHover");
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

void Skin::DrawVerticalEdgedRectElement(const EdgedRectElement &element, const Point &pos, const Point &size) const
{
	const float height = element.edgeWidth;

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);

	va.Add(vector3f(pos.x+size.x, pos.y,               0.0f), scaled(vector2f(element.pos.x+element.size.x, element.pos.y)));
	va.Add(vector3f(pos.x,        pos.y,               0.0f), scaled(vector2f(element.pos.x,                element.pos.y)));
	va.Add(vector3f(pos.x+size.x, pos.y+height,        0.0f), scaled(vector2f(element.pos.x+element.size.x, element.pos.y+height)));
	va.Add(vector3f(pos.x,        pos.y+height,        0.0f), scaled(vector2f(element.pos.x,                element.pos.y+height)));
	va.Add(vector3f(pos.x+size.x, pos.y+size.y-height, 0.0f), scaled(vector2f(element.pos.x+element.size.x, element.pos.y+element.size.y-height)));
	va.Add(vector3f(pos.x,        pos.y+size.y-height, 0.0f), scaled(vector2f(element.pos.x,                element.pos.y+element.size.y-height)));
	va.Add(vector3f(pos.x+size.x, pos.y+size.y,        0.0f), scaled(vector2f(element.pos.x+element.size.x, element.pos.y+element.size.y)));
	va.Add(vector3f(pos.x,        pos.y+size.y,        0.0f), scaled(vector2f(element.pos.x,                element.pos.y+element.size.y)));

	m_renderer->DrawTriangles(&va, m_material.Get(), Graphics::TRIANGLE_STRIP);
}

void Skin::DrawHorizontalEdgedRectElement(const EdgedRectElement &element, const Point &pos, const Point &size) const
{
	const float width = element.edgeWidth;

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);

	va.Add(vector3f(pos.x,              pos.y,        0.0f), scaled(vector2f(element.pos.x,                      element.pos.y)));
	va.Add(vector3f(pos.x,              pos.y+size.y, 0.0f), scaled(vector2f(element.pos.x,                      element.pos.y+element.size.y)));
	va.Add(vector3f(pos.x+width,        pos.y,        0.0f), scaled(vector2f(element.pos.x+width,                element.pos.y)));
	va.Add(vector3f(pos.x+width,        pos.y+size.y, 0.0f), scaled(vector2f(element.pos.x+width,                element.pos.y+element.size.y)));
	va.Add(vector3f(pos.x+size.x-width, pos.y,        0.0f), scaled(vector2f(element.pos.x+element.size.x-width, element.pos.y)));
	va.Add(vector3f(pos.x+size.x-width, pos.y+size.y, 0.0f), scaled(vector2f(element.pos.x+element.size.x-width, element.pos.y+element.size.y)));
	va.Add(vector3f(pos.x+size.x,       pos.y,        0.0f), scaled(vector2f(element.pos.x+element.size.x,       element.pos.y)));
	va.Add(vector3f(pos.x+size.x,       pos.y+size.y, 0.0f), scaled(vector2f(element.pos.x+element.size.x,       element.pos.y+element.size.y)));

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
	return BorderedRectElement(v[0], v[1], v[2], v[3], v[4]*m_scale);
}

Skin::EdgedRectElement Skin::LoadEdgedRectElement(const std::string &spec)
{
	std::vector<int> v(5);
	SplitSpec(spec, v);
	return EdgedRectElement(v[0], v[1], v[2], v[3], v[4]);
}

}
