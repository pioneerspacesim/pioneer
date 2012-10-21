
#include "DistanceFieldFont.h"
#include "graphics/Texture.h"
#include "graphics/VertexArray.h"
#include "utils.h"
#include "FileSystem.h"
#include "StringRange.h"
#include <sstream>
#include <iostream>

namespace Text {

static const vector2f s_fontGeomSize(0.8f, 1.2f);

DistanceFieldFont::DistanceFieldFont(const std::string &definition, Graphics::Texture *tex)
: m_texture(tex)
, m_sheetSize(0.f)
{
	//parse definition
	RefCountedPtr<FileSystem::FileData> fontdef = FileSystem::gameDataFiles.ReadFile(definition);
	StringRange data = fontdef->AsStringRange();

	bool doingCharacters = false;

	while (!data.Empty()) {
		const StringRange line = data.ReadLine();
		if (line.Empty()) continue;

		if (doingCharacters) {
			ParseChar(line);
		} else {
			if (starts_with(line.begin, "common ")) //contains only non-char data we care about right now
				ParseCommon(line);
			else if (starts_with(line.begin, "chars ")) //after this, file should be all characters
				doingCharacters = true;
		}
	}	
}

void DistanceFieldFont::GetGeometry(Graphics::VertexArray &va, const std::string &text, const vector2f &offset)
{
	assert(va.HasAttrib(Graphics::ATTRIB_NORMAL) && va.HasAttrib(Graphics::ATTRIB_UV0));
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

// create a preferred format vertex array
Graphics::VertexArray *DistanceFieldFont::CreateVertexArray() const
{
	return new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL | Graphics::ATTRIB_UV0);
}

void DistanceFieldFont::AddGlyph(Graphics::VertexArray &va, const vector2f &pos, const Glyph& g)
{
	vector3f n(0.f, 0.f, -1.f);
	const vector2f &uv = g.uv; //uv offset
	const float uWidth = g.uvSize.x;
	const float vHeight = g.uvSize.y;
	const float w = g.size.x;
	const float h = g.size.y;
	va.Add(vector3f(pos.x, pos.y, 0.f), n, vector2f(uv.x, uv.y+vHeight));
	va.Add(vector3f(pos.x + w, pos.y, 0.f), n, vector2f(uv.x+uWidth, uv.y+vHeight));
	va.Add(vector3f(pos.x, pos.y + h, 0.f), n, vector2f(uv.x, uv.y));

	va.Add(vector3f(pos.x, pos.y + h, 0.f), n, vector2f(uv.x, uv.y));
	va.Add(vector3f(pos.x + w, pos.y, 0.f), n, vector2f(uv.x+uWidth, uv.y+vHeight));
	va.Add(vector3f(pos.x + w, pos.y + h, 0.f), n, vector2f(uv.x+uWidth, uv.y));
}

//split key=value
static bool split_token(const std::string &t, std::pair<std::string, std::string> &out)
{
	size_t pos = t.find_first_of("=");

	if (pos == std::string::npos) return false;

	out.first = t.substr(0, pos);
	out.second = t.substr(pos+1);

	return true;
}

template <typename T>
T get_value(const std::string& in)
{
	//should check for quotes...
	//int startpos = txt.find("\"")+1;
	//int endpos = txt.rfind("\"");
	//double value = atof(txt.substr(startpos, endpos-startpos).c_str());
	//return value;
	T out;
	std::stringstream ss(in);
	ss >> out;
	return out;
}

//get font definitions from a line of xml, insert glyph information into the map
void DistanceFieldFont::ParseChar(const StringRange &r)
{
	//assumed format:
	//char id=0 x=251.75 y=0 width=2 height=2.125 xoffset=-1 yoffset=30.875 xadvance=16 page=0 chnl=15

	std::stringstream ss(r.ToString());
	std::string token;

	Uint32 id = 0;
	double x = 0.0;
	double y = 0.0;
	double uSize = 0.0;
	double vSize = 0.0;

	while (ss >> token != 0) {
			std::pair<std::string, std::string> pair;
			split_token(token, pair);

			//only care about some values
			if (pair.first == "id")
				id = get_value<Uint32>(pair.second);
			else if (pair.first == "x")
				x = get_value<double>(pair.second);
			else if (pair.first == "y")
				y = get_value<double>(pair.second);
			else if (pair.first == "width")
				uSize = get_value<double>(pair.second);
			else if (pair.first == "height")
				vSize = get_value<double>(pair.second);
	}

	Glyph g;
	g.uv = vector2f(float(x)/m_sheetSize.x, float(y)/m_sheetSize.y);
	g.uvSize = vector2f(float(uSize)/m_sheetSize.x, float(vSize)/m_sheetSize.y);
	g.size = s_fontGeomSize;
	m_glyphs[id] = g;
}

void DistanceFieldFont::ParseCommon(const StringRange &line)
{
	std::stringstream ss(line.ToString());
	std::string token;

	while (ss >> token != 0) {
		std::pair<std::string, std::string> pair;
		split_token(token, pair);
		if (pair.first == "scaleW")
			m_sheetSize.x = get_value<float>(pair.second);
		else if (pair.first == "scaleH")
			m_sheetSize.y = get_value<float>(pair.second);
	}
}

}