// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXT_TEXTUREFONT_H
#define _TEXT_TEXTUREFONT_H

#include "libs.h"
#include "Font.h"
#include "graphics/Texture.h"
#include "graphics/Material.h"
#include "graphics/VertexArray.h"
#include "graphics/RenderState.h"

namespace Text {

class TextureFont : public Font {

public:
	TextureFont(const FontDescriptor &descriptor, Graphics::Renderer *renderer);

	void RenderString(const char *str, float x, float y, const Color &color = Color::WHITE);
	Color RenderMarkup(const char *str, float x, float y, const Color &color = Color::WHITE);
	void MeasureString(const char *str, float &w, float &h);
	void MeasureCharacterPos(const char *str, int charIndex, float &x, float &y) const;
	int PickCharacter(const char *str, float mouseX, float mouseY) const;

	// general baseline-to-baseline height
	float GetHeight() const { return m_height; }
	// general descender height
	float GetDescender() const { return m_descender; }

	enum { MAX_FAST_GLYPHS = 256 };

	struct glfglyph_t {
		float advx, advy;
		float width, height;
		float texWidth, texHeight;
		int offx, offy;
		float offU, offV; //atlas UV offset
		Uint32 ftIndex;
	};
	const glfglyph_t &GetGlyph(Uint32 ch) const { return ch < MAX_FAST_GLYPHS ? m_glyphsFast[ch] : m_glyphs.find(ch)->second; }

	static int GetGlyphCount() { return s_glyphCount; }
	static void ClearGlyphCount() { s_glyphCount = 0; }

	// fill a vertex array with single-colored text
	void CreateGeometry(Graphics::VertexArray &, const char *str, float x, float y, const Color &color = Color::WHITE);
	RefCountedPtr<Graphics::Texture> GetTexture() { return m_texture; }

private:
	Graphics::Renderer *m_renderer;

	void AddGlyphGeometry(Graphics::VertexArray *va, const glfglyph_t &glyph, float x, float y, const Color &color);
	float m_height;
	float m_descender;
	RefCountedPtr<Graphics::Texture> m_texture;
	std::unique_ptr<Graphics::Material> m_mat;
	Graphics::VertexArray m_vertices;
	Graphics::RenderState *m_renderState;

	static int s_glyphCount;

	std::vector<glfglyph_t> m_glyphsFast; // for fast lookup of low-index glyphs
	std::map<Uint32,glfglyph_t> m_glyphs;

	static const Uint32 CHARACTER_RANGES[];
};

}

#endif
