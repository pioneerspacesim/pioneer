#include "Label3D.h"
#include "NodeVisitor.h"

namespace Newmodel {

struct Glyph {
	Glyph() : uv(0.f) { }
	Glyph(const vector2f &offs) : uv(offs) { }
	vector2f uv;
};

static std::map<char, Glyph> s_characters;
static const float s_width = 4.f;
static const float s_height = 4.f;
static const float s_charW = 1.f/16.f;
static const float s_charH = 1.f/8.f;
static const int s_texWidth = 512;
static const int s_texHeight = 256;

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
	int row = 0;
	int col = 0;
	for(char i=32; i<127; i++) {
		s_characters[i] = Glyph(vector2f(col * s_charW, row * s_charH));
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

void Label3D::Accept(NodeVisitor &nv)
{
	nv.ApplyLabel(*this);
}

static void add_char(Graphics::VertexArray &va, const vector2f &pos, const vector2f &uv, const Color &c)
{
	const float w = s_width;
	const float h = s_height;
	const float u = uv.x;
	const float v = uv.y;

	va.Add(vector3f(pos.x, pos.y, 0.f), c, vector2f(uv.x, uv.y+s_charH));
	va.Add(vector3f(pos.x + w, pos.y, 0.f), c, vector2f(uv.x+s_charW, uv.y+s_charH));
	va.Add(vector3f(pos.x, pos.y + h, 0.f), c, vector2f(uv.x, uv.y));

	va.Add(vector3f(pos.x, pos.y + h, 0.f), c, vector2f(uv.x, uv.y));
	va.Add(vector3f(pos.x + w, pos.y, 0.f), c, vector2f(uv.x+s_charW, uv.y+s_charH));
	va.Add(vector3f(pos.x + w, pos.y + h, 0.f), c, vector2f(uv.x+s_charW, uv.y));
}

void Label3D::CreateGeometry(Graphics::VertexArray& va, const std::string& text, const Color &c)
{
	const float spacing = 1.5;
	const float yoff = -s_height/2.f;
	const float xoff = (text.length() * spacing) / 2.f;
	for(unsigned int i=0; i<text.length(); i++) {
		const char chr = text.at(i);
		std::map<char, Glyph>::const_iterator it = s_characters.find(chr);
		if (it != s_characters.end()) {
			const Glyph &glyph = it->second;
			add_char(va, vector2f(i * spacing - xoff, yoff), glyph.uv, c);
		}
	}
}

}