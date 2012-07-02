#include "DistanceFieldFont.h"
#include "graphics/Texture.h"
#include "FileSystem.h"
#include "StringRange.h"
#include <sstream>
#include <iostream>

namespace Text {

static const vector2f s_fontGeomSize(0.8f, 1.2f);
static const vector2f s_texSize(256.f, 128.f);

DistanceFieldFont::DistanceFieldFont(const std::string &definition, Graphics::Texture *tex)
: m_texture(tex)
{
	//parse definition
	RefCountedPtr<FileSystem::FileData> fontdef = FileSystem::gameDataFiles.ReadFile(definition);
	StringRange data = fontdef->AsStringRange();
	while (!data.Empty()) {
		ParseLine(data.ReadLine());
	}	
}

void DistanceFieldFont::GetGeometry(Graphics::VertexArray &va, const std::string &text, const vector2f &offset)
{
	//using monospaced font
	const float spacing = s_fontGeomSize.x;
	vector2f offs(
		text.length() * spacing * -0.5f,
		-s_fontGeomSize.y/2.f
	);
	for(unsigned int i=0; i<text.length(); i++) {
		Uint32 chr = Uint32(text.at(i));
		std::map<Uint32, Glyph>::const_iterator it = m_glyphs.find(chr);
		if (it != m_glyphs.end()) {
			const Glyph &glyph = it->second;
			AddGlyph(va, vector2f(i * spacing, 0.f) + offs, glyph);
		}
	}
}

//extract a quoted value
static double get_value(const std::string &txt)
{
	int startpos = txt.find("\"")+1;
	int endpos = txt.rfind("\"");
	double value = atof(txt.substr(startpos, endpos-startpos).c_str());
	return value;
}

//get font definitions from a line of xml, insert glyph information into the map
void DistanceFieldFont::ParseLine(const StringRange &r)
{
	if(r.Empty()) return;

	//I don't know how to use stringrange, so...
	bool definition = false;
	std::stringstream ss(r.ToString());
	std::string token;

	Uint32 id = 0;
	double x = 0.0;
	double y = 0.0;
	double uSize = 0.0;
	double vSize = 0.0;

	while (ss >> token != 0) {
		if (token.compare("<char") == 0) {
			definition = true;
		} else if(definition) {
			if (token.compare("/>") == 0)
				continue;
			else if (starts_with(token, "id="))
				id = Uint32(get_value(token));
			else if (starts_with(token, "x="))
				x = get_value(token);
			else if (starts_with(token, "y="))
				y = get_value(token);
			else if (starts_with(token, "width="))
				uSize = get_value(token);
			else if (starts_with(token, "height="))
				vSize = get_value(token);
		}
	}

	if (definition) {
		Glyph g;
		g.uv = vector2f(float(x)/s_texSize.x, float(y)/s_texSize.y);
		g.uvSize = vector2f(float(uSize)/s_texSize.x, float(vSize)/s_texSize.y);
		g.size = s_fontGeomSize;
		m_glyphs[id] = g;
	}
}

void DistanceFieldFont::AddGlyph(Graphics::VertexArray &va, const vector2f &pos, const Glyph& g)
{
	const Color4f c = Color::RED; //XXX
	const vector2f &uv = g.uv; //uv offset
	const float uWidth = g.uvSize.x;
	const float vHeight = g.uvSize.y;
	const float w = g.size.x;
	const float h = g.size.y;
	va.Add(vector3f(pos.x, pos.y, 0.f), c, vector2f(uv.x, uv.y+vHeight));
	va.Add(vector3f(pos.x + w, pos.y, 0.f), c, vector2f(uv.x+uWidth, uv.y+vHeight));
	va.Add(vector3f(pos.x, pos.y + h, 0.f), c, vector2f(uv.x, uv.y));

	va.Add(vector3f(pos.x, pos.y + h, 0.f), c, vector2f(uv.x, uv.y));
	va.Add(vector3f(pos.x + w, pos.y, 0.f), c, vector2f(uv.x+uWidth, uv.y+vHeight));
	va.Add(vector3f(pos.x + w, pos.y + h, 0.f), c, vector2f(uv.x+uWidth, uv.y));
}

}