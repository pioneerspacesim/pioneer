#include "newmodel/Label3D.h"

namespace Newmodel {

struct Glyph {
	Glyph() : uv(0.f) { }
	Glyph(const vector2f &offs) : uv(offs) { }
	vector2f uv;
};

static std::map<char, Glyph> s_characters;

Label3D::Label3D(Graphics::Texture *font)
: Node(NODE_TRANSPARENT)
{
	m_geometry.Reset(new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0));
	m_material.Reset(new Graphics::Material);
	m_material->texture0 = font;
	m_material->twoSided = true;
	m_material->diffuse = Color::WHITE;

	//XXX this is very unsatisfying. I'd rather export the offset values
	//using the same tool that generates the font texture
	const int texWidth = 512;
	const int texHeight = 256;
	const float charW = 1.f/16.f;
	const float charH = 1.f/8.f;

	int row = 0;
	int col = 0;
	for(char i=32; i<127; i++) {
		s_characters[i] = Glyph(vector2f(col * charW, row * charH));
		col++;
		if (col >= 16) {
			row++;
			col = 0;
		}
	}
}

void Label3D::SetText(const std::string &text)
{
	//regenerate geometry
	m_geometry->Clear();
	CreateGeometry(*m_geometry.Get(), text, Color::RED);
}

void Label3D::Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd)
{
	//needs alpha test!
	r->SetTransform(trans);
	r->DrawTriangles(m_geometry.Get(), m_material.Get());
}

static void add_char(Graphics::VertexArray &va, const vector2f &pos, const vector2f &uv, const Color &c)
{
	const float w = 4.f;
	const float h = 4.f;
	const float charW = 1.f/16.f;
	const float charH = 1.f/8.f;
	const float u = uv.x;
	const float v = uv.y;
	va.Add(vector3f(pos.x + w, pos.y + h, 0.f), c, vector2f(u+charW, v+charH));
	va.Add(vector3f(pos.x + w, pos.y, 0.f), c, vector2f(u+charW, v));
	va.Add(vector3f(pos.x, pos.y, 0.f), c, vector2f(u, v));

	va.Add(vector3f(pos.x, pos.y, 0.f), c, vector2f(u, v));
	va.Add(vector3f(pos.x, h, 0.f), c, vector2f(u, v+charH));
	va.Add(vector3f(pos.x + w, pos.y + h, 0.f), c, vector2f(u+charW, v+charH));
}

void Label3D::CreateGeometry(Graphics::VertexArray& va, const std::string& text, const Color &c)
{
	const float spacing = 1.5f;
	for(unsigned int i=0; i<text.length(); i++) {
		const char chr = text.at(i);
		std::map<char, Glyph>::const_iterator it = s_characters.find(chr);
		if (it != s_characters.end()) {
			const Glyph &glyph = it->second;
			add_char(va, vector2f(i * spacing, 0.f), glyph.uv, c);
		}
	}
}

}