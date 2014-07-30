// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXT_TEXTUREFONT_H
#define _TEXT_TEXTUREFONT_H

#include "libs.h"
#include "FontConfig.h"
#include "RefCounted.h"
#include "graphics/Texture.h"
#include "graphics/Material.h"
#include "graphics/VertexArray.h"
#include "graphics/RenderState.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

namespace FileSystem { class FileData; }

namespace Text {

class TextureFont : public RefCounted {

public:
	TextureFont(const FontConfig &config, Graphics::Renderer *renderer, float scale = 1.0f);
	~TextureFont();

	void RenderString(const char *str, float x, float y, const Color &color = Color::WHITE);
	Color RenderMarkup(const char *str, float x, float y, const Color &color = Color::WHITE);
	void MeasureString(const char *str, float &w, float &h);
	void MeasureCharacterPos(const char *str, int charIndex, float &x, float &y);
	int PickCharacter(const char *str, float mouseX, float mouseY);

	// general baseline-to-baseline height
	float GetHeight() const { return m_height; }
	// general descender height
	float GetDescender() const { return m_descender; }

	struct Glyph {
		Glyph() : advX(0), advY(0), width(0), height(0), texWidth(0), texHeight(0), offX(0), offY(0), offU(0), offV(0), ftIndex(0) {}
		float advX, advY;
		float width, height;
		float texWidth, texHeight;
		int offX, offY;
		float offU, offV; //atlas UV offset
		FT_Face ftFace;
		Uint32 ftIndex;
	};
	const Glyph &GetGlyph(Uint32 ch);

	static int GetGlyphCount() { return s_glyphCount; }
	static void ClearGlyphCount() { s_glyphCount = 0; }

	// fill a vertex array with single-colored text
	void CreateGeometry(Graphics::VertexArray &, const char *str, float x, float y, const Color &color = Color::WHITE);
	RefCountedPtr<Graphics::Texture> GetTexture() { return m_texture; }

private:
	TextureFont(const TextureFont &);
	TextureFont &operator=(const TextureFont &);

	FontConfig m_config;
	Graphics::Renderer *m_renderer;
	float m_scale;

	FT_Library m_ftLib;
	FT_Stroker m_stroker;

	FT_Face GetFTFace(const FontConfig::Face &face);
	std::map<FontConfig::Face,std::pair<FT_Face,RefCountedPtr<FileSystem::FileData>>> m_faces;

	Glyph BakeGlyph(Uint32 chr);

	float GetKern(const Glyph &a, const Glyph &b);

	void AddGlyphGeometry(Graphics::VertexArray *va, const Glyph &glyph, float x, float y, const Color &color);
	float m_height;
	float m_descender;
	std::unique_ptr<Graphics::Material> m_mat;
	Graphics::VertexArray m_vertices;
	Graphics::RenderState *m_renderState;

	static int s_glyphCount;

	std::map<Uint32,Glyph> m_glyphs;

	// UV offsets for glyphs
	int m_atlasU;
	int m_atlasV;
	int m_atlasVIncrement;

	RefCountedPtr<Graphics::Texture> m_texture;
	Graphics::TextureFormat m_texFormat;

	std::vector<unsigned char> m_buf;
	int m_bufWidth, m_bufHeight;
	int m_bpp;
};

}

#endif
