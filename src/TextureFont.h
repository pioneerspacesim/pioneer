#ifndef _TEXTUREFONT_H
#define _TEXTUREFONT_H

#include "Font.h"
#include "Color.h"
#include "graphics/Texture.h"
#include "graphics/Material.h"

namespace Graphics {
	class Renderer;
	class VertexArray;
}

class TextureFont : public Font {

public:
	TextureFont(const FontConfig &fc);

	void RenderString(Graphics::Renderer *r, const char *str, float x, float y, const Color &color = Color::WHITE);
	Color RenderMarkup(Graphics::Renderer *r, const char *str, float x, float y, const Color &color = Color::WHITE);
	void MeasureString(const char *str, float &w, float &h);
	void MeasureCharacterPos(const char *str, int charIndex, float &x, float &y) const;
	int PickCharacter(const char *str, float mouseX, float mouseY) const;

	// of Ms
	float GetHeight() const { return m_height; }
	float GetWidth() const { return m_width; }
	float GetDescender() const { return m_descender; }
	struct glfglyph_t {
		float advx, advy;
		float width, height;
		float texWidth, texHeight;
		int offx, offy;
		float offU, offV; //atlas UV offset
	};
	const glfglyph_t &GetGlyph(Uint32 ch) { return m_glyphs[ch]; }

	static int GetGlyphCount() { return s_glyphCount; }
	static void ClearGlyphCount() { s_glyphCount = 0; }

private:
	void AddGlyphGeometry(Graphics::VertexArray *va, Uint32 chr, float x, float y, const Color &color);
	float m_height;
	float m_width;
	float m_descender;
	int m_texSize, m_pixSize;
	RefCountedPtr<Graphics::Texture> m_texture;
	Graphics::Material m_mat;

	static int s_glyphCount;
	std::map<Uint32,glfglyph_t> m_glyphs;
	static const Uint32 m_firstCharacter = 0x20; //32
	static const Uint32 m_lastCharacter = 0x1ff; //511
};

#endif
