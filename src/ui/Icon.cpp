// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Icon.h"
#include "Context.h"
#include "FileSystem.h"
#include "graphics/TextureBuilder.h"

static const char CONFIG_FILE[] = "ui/Icons.ini";
static const char FALLBACK_ICON[] = "Blank";

namespace UI {

IniConfig Icon::s_config;
RefCountedPtr<Graphics::Texture> Icon::s_texture;
vector2f Icon::s_texScale;
RefCountedPtr<Graphics::Material> Icon::s_material;

// XXX copypasta'd from Skin.cpp. this whole texture atlas and rectangles and
// whatever should be abstracted out
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

Icon::Icon(Context *context, const std::string &iconName): Widget(context),
	m_color(Color::WHITE)
{
	if (!s_texture) {
		s_config.Read(FileSystem::gameDataFiles, CONFIG_FILE);

		s_texture.Reset(Graphics::TextureBuilder::UI(s_config.String("TextureFile")).GetOrCreateTexture(GetContext()->GetRenderer(), "ui"));

		const Graphics::TextureDescriptor &texDesc = s_texture->GetDescriptor();
		s_texScale = vector2f(1.0f/texDesc.dataSize.x, 1.0f/texDesc.dataSize.y);

		Graphics::MaterialDescriptor matDesc;
		matDesc.textures = 1;
		s_material.Reset(GetContext()->GetRenderer()->CreateMaterial(matDesc));
		s_material->texture0 = s_texture.Get();
	}

	std::string spec(s_config.String(iconName.c_str()));
	if (spec.size() == 0)
		spec = s_config.String(FALLBACK_ICON);
	assert(spec.size() > 0);

	std::vector<int> v(2);
	SplitSpec(spec, v);
	m_texPos = Point(v[0], v[1]);
}

Point Icon::PreferredSize()
{
	SetSizeControlFlags(NO_HEIGHT | PRESERVE_ASPECT);
	return Point(48);
}

void Icon::Draw()
{
	const Point &offset = GetActiveOffset();
	const Point &area = GetActiveArea();

	const float x = offset.x;
	const float y = offset.y;
	const float sx = area.x;
	const float sy = area.y;

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
	va.Add(vector3f(x,    y,    0.0f), vector2f(s_texScale.x*(m_texPos.x),    s_texScale.y*(m_texPos.y)));
	va.Add(vector3f(x,    y+sy, 0.0f), vector2f(s_texScale.x*(m_texPos.x),    s_texScale.y*(m_texPos.y+48)));
	va.Add(vector3f(x+sx, y,    0.0f), vector2f(s_texScale.x*(m_texPos.x+48), s_texScale.y*(m_texPos.y)));
	va.Add(vector3f(x+sx, y+sy, 0.0f), vector2f(s_texScale.x*(m_texPos.x+48), s_texScale.y*(m_texPos.y+48)));

	Graphics::Renderer *r = GetContext()->GetRenderer();
	s_material->diffuse = m_color;
	s_material->diffuse = Color(m_color.r, m_color.g, m_color.b, GetContext()->GetOpacity()*m_color.a);
	auto renderState = GetContext()->GetSkin().GetAlphaBlendState();
	r->DrawTriangles(&va, renderState, s_material.Get(), Graphics::TRIANGLE_STRIP);
}

}
