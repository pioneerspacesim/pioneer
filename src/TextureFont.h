#ifndef _TEXTUREFONT_H
#define _TEXTUREFONT_H

#include "Font.h"
#include "FontConfig.h"
#include "Texture.h"

class TextureFont : public Font {

private:

	class GlyphTexture : public Texture {
	public:
		GlyphTexture(Uint8 *data, int width, int height);
		virtual void Bind();
	};

public:
	TextureFont(FontManager &fm, const std::string &config_filename);
	~TextureFont();

	void RenderString(const char *str, float x, float y);
	void RenderMarkup(const char *str, float x, float y);
	void MeasureString(const char *str, float &w, float &h);
	void MeasureCharacterPos(const char *str, int charIndex, float &x, float &y) const;
	int PickCharacter(const char *str, float mouseX, float mouseY) const;

	// of Ms
	float GetHeight() const { return m_height; }
	float GetWidth() const { return m_width; }
	float GetDescender() const { return m_descender; }
	struct glfglyph_t {
		GlyphTexture *texture;
		float advx, advy;
		float width, height;
		int offx, offy;
	};
	const glfglyph_t &GetGlyph(Uint32 ch) { return m_glyphs[ch]; }

	static int GetGlyphCount() { return s_glyphCount; }
	static void ClearGlyphCount() { s_glyphCount = 0; }

private:
	void RenderGlyph(Uint32 chr, float x, float y);
	float m_height;
	float m_width;
	float m_descender;
	int m_texSize, m_pixSize;

	static int s_glyphCount;
	std::map<Uint32,glfglyph_t> m_glyphs;
};

#endif
