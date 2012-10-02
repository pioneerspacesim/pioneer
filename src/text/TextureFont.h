// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXT_TEXTUREFONT_H
#define _TEXT_TEXTUREFONT_H

#include "Font.h"
#include "Color.h"
#include "graphics/Texture.h"
#include "graphics/Material.h"
#include "graphics/VertexArray.h"
#include <map>

namespace Graphics {
	class Material;
	class Renderer;
}

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

	struct glfglyph_t {
		float advx, advy;
		float width, height;
		float texWidth, texHeight;
		int offx, offy;
		float offU, offV; //atlas UV offset
	};
	const glfglyph_t &GetGlyph(Uint32 ch) const { return m_glyphs.find(ch)->second; }

	static int GetGlyphCount() { return s_glyphCount; }
	static void ClearGlyphCount() { s_glyphCount = 0; }

private:
	Graphics::Renderer *m_renderer;

	void AddGlyphGeometry(Graphics::VertexArray *va, Uint32 chr, float x, float y, const Color &color);
	float m_height;
	float m_descender;
	int m_texSize;
	RefCountedPtr<Graphics::Texture> m_texture;
	ScopedPtr<Graphics::Material> m_mat;
	Graphics::VertexArray m_vertices;

	static int s_glyphCount;
	std::map<Uint32,glfglyph_t> m_glyphs;

	static const Uint32 s_firstCharacter = 0x20; //32
	static const Uint32 s_lastCharacter = 0x1ff; //511
};

}

#endif
